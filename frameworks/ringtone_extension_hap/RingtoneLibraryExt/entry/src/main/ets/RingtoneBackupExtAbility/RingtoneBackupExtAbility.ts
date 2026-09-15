/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import BackupExtensionAbility, {BundleVersion} from '@ohos.application.BackupExtensionAbility';
import fs from '@ohos.file.fs';
// @ts-ignore
import ringtonerestore from '@ohos.multimedia.ringtonerestore';

const TAG = 'RingtoneBackupExtAbility';

const ringtonePath = '/data/storage/el2/base/files/Ringtone/';

const RESTORE_SCENE_TYPE_DUAL_UPGRADE : number = 0;
const RESTORE_SCENE_TYPE_SINGLE_CLONE : number = 1;
const RESTORE_SCENE_TYPE_DUAL_CLONE : number = 2;

const UPGRADE_NAME = '0.0.0.0';
const DUAL_FRAME_CLONE_NAME = '99.99.99.999';

const ON_RESTORE_COMMON_CODE : string = '0';
const ON_RESTORE_ERROR_CODE : string = '13500099';

export interface IResultExInfo {
  resultInfo: [IResultErrorInfo];
}

export interface IResultErrorInfo {
  type: string;
  errorCode: string;
  errorInfo: string;
}

interface DetailData {
  version: number;
  peerSlotNum: number;
}

interface BackupInfoList {
  type: string;
  detail: string;
}

export default class RingtoneBackupExtAbility extends BackupExtensionAbility {
  async onBackup() : Promise<void> {
    console.log(TAG, 'onBackup ok.');
  }

  async onBackupEx(backupInfo: string): Promise<string> {
    console.log(TAG, `onBackupEx start, backupInfo: ${backupInfo}`);
    try {
      let needEsimClean : boolean = false;
      let peerSlotNum = 2;
      if (backupInfo === '') {
        // no need esim clean  
      } else {
        let backupInfoObj : object = JSON.parse(backupInfo);
        let backupInfoList : BackupInfoList[] = backupInfoObj as BackupInfoList[];
        for (let i = 0; i < backupInfoList.length; i++) {
          let info = backupInfoList[i];
          let type = info['type'];
          if (type !== 'compatibility_info') {
            continue;
          }
          console.log(TAG, `backupInfo-type: ${type}`);
          let detailData : DetailData = JSON.parse(info['detail']) as DetailData;
          console.log(TAG, `backupInfo-detailData.peerSlotNum: ${detailData.peerSlotNum}`);
          peerSlotNum = detailData.peerSlotNum;
        }
        console.log(TAG, `peerSlotNum: ${peerSlotNum}`);
      }

      const tempDbPath = '/storage/media/local/files/.backup/backup/ringtone_temp_rdb/';
      const srcDbPath = '/data/storage/el1/database/rdb';

      if (!this.isDirExist(srcDbPath)) {
        console.log(TAG, 'source db path not exist, skip');
        return '';
      }

      const ringtoneBasePath = '/data/storage/el2/base/files/Ringtone/';
      // peerSlotNum: 传入实际值用于eSIM裁剪；ringtoneBasePath用于过滤未使用自定义铃声
      await this.copyDbAndClean(srcDbPath, tempDbPath, peerSlotNum, ringtoneBasePath);
      const retStr = JSON.stringify({ compatibleDirMapping : ["/storage/media/local/files/.backup/backup/ringtone_temp_rdb/", "/data/storage/el1/database/rdb/"]});
      console.log(TAG, `onBackupEx end, retStr: ${retStr}`);
      return retStr;
    } catch (err) {
      console.error(TAG, `onBackupEx error: ${err.message}`);
      return '';
    }
  }

  private async copyDbAndClean(srcPath: string, destPath: string, peerSlotNum: number,
    ringtoneBasePath: string): Promise<void> {
    console.log(TAG, `copyDbAndClean from ${srcPath} to ${destPath}, peerSlotNum: ${peerSlotNum}`);
    try {
      const dbPath = destPath + 'ringtone_library.db';
      const cleanResult: number = await ringtonerestore.cleanESimData(dbPath, peerSlotNum, ringtoneBasePath);
      console.log(TAG, `cleanESimData result: ${cleanResult}`);
      console.log(TAG, `copyDbAndClean completed successfully`);
    } catch (err) {
      console.error(TAG, `copyDbAndClean error: ${err.message}, code: ${err.code}`);
    }
  }

  async getRestoreCompatibilityInfo(extInfo: string): Promise<string> {
    console.log(TAG, `getRestoreCompatibilityInfo, extInfo: ${extInfo}`);
    try {
      let compatibilityInfo = {
        version: 1,
        peerSlotNum: 4
      };
      let ret = JSON.stringify(compatibilityInfo);
      console.log(TAG, `getRestoreCompatibilityInfo ret: ${ret}`);
      return ret;
    } catch (error) {
      console.error(TAG, `getRestoreCompatibilityInfo failed with error. Code: ${error.code}, message: ${error.message}`);
    }
    return '';
  }

  async onRestoreEx(bundleVersion: BundleVersion, restoreInfo: string): Promise<string> {
    console.log(TAG, `onRestoreEx ok ${JSON.stringify(bundleVersion)}`);
    console.time(TAG + ' RESTORE');
    const backupBasePath = this.context.backupDir + 'restore';
    const backupFilePath = backupBasePath + '/data/storage/el2/base/files/Ringtone/';

    console.log(TAG, `onRestoreEx ok ${JSON.stringify(backupFilePath)}`);
    let srcPath:string = backupFilePath;
    let destPath:string = ringtonePath;
    let cloneType:number;
    if (bundleVersion.name.startsWith(UPGRADE_NAME)) {
      cloneType = RESTORE_SCENE_TYPE_DUAL_UPGRADE;
    } else if (bundleVersion.name === DUAL_FRAME_CLONE_NAME && bundleVersion.code === 0) {
      cloneType = RESTORE_SCENE_TYPE_DUAL_CLONE;
    } else {
      cloneType = RESTORE_SCENE_TYPE_SINGLE_CLONE;
    }
    let restoreResult:number = await ringtonerestore.startRestore(cloneType, backupBasePath);
    console.log(TAG, `restoreResult:${restoreResult}`);
    console.timeEnd(TAG + ' RESTORE');
    console.time(TAG + ' MOVE REST FILES');
    await this.moveRestFiles(srcPath, destPath);
    console.timeEnd(TAG + ' MOVE REST FILES');
    let result:boolean = restoreResult === 0;
    let resultExInfo: IResultExInfo = {
      resultInfo: [
        {
          type: 'ErrorInfo',
          errorCode: result ? ON_RESTORE_COMMON_CODE : ON_RESTORE_ERROR_CODE, // 如果成功，错误码返回0，如果失败，返回约定的错误码
          errorInfo: '',
        }
      ]
    };

    let resultInfo: string = JSON.stringify(resultExInfo);
    console.log(TAG, `restore end resultInfo:${resultInfo}`);
    return resultInfo;
  }

  private isDirExist(dirPath : string) : boolean {
    try {
      let stat = fs.lstatSync(dirPath);
      return stat.isDirectory();
    } catch (err) {
      console.error(TAG, `lstatSync failed, message = ${err.message}; code = ${err.code}`);
      return false;
    }
  }

  private isFileExist(filePath : string) : boolean {
    try {
      return fs.accessSync(filePath);
    } catch (err) {
      console.error(TAG, `accessSync failed, message = ${err.message}; code = ${err.code}`);
      return false;
    }
  }

  private async moveRestFiles(srcPath : string, destPath : string) : Promise<void> {
    console.log(TAG, 'Start to move rest files.');
    const MOVE_ERR_CODE = 13900015;
    let conflictList = [];
    try {
      console.log(TAG, `move dir`);
      await fs.copyDir(srcPath, destPath, 0);
    } catch (err) {
      console.log(TAG, `catch err: ${JSON.stringify(err)}`);
      if (err.code === MOVE_ERR_CODE) {
        conflictList = err.data;
      } else {
        console.error(TAG, `move directory failed, message = ${err.message}, code = ${err.code}`);
      }
    }
    for (let i = 0; i < conflictList.length; i++) {
      console.log(TAG, `move conflect file from [${conflictList[i].srcFile}] to [${conflictList[i].destFile}]`);
      try {
        await this.moveConflictFile(conflictList[i].srcFile, conflictList[i].destFile);
      } catch (err) {
        console.error(TAG, `MoveConflictFile failed, message = ${err.message}, code = ${err.code}`);
      }
    }
  }

  private async moveConflictFile(srcFile : string, dstFile : string) : Promise<void> {
    let srcStat = fs.statSync(srcFile);
    let dstStat = fs.statSync(dstFile);
    if (srcStat.size === dstStat.size) {
      console.log(TAG, `conflictFile is same, srcFile: [${srcFile}] size: (${srcStat.size}), dstFile: [${dstFile}] size: (${dstStat.size}), return.`);
      return;
    }
    const srcArr = srcFile.split('/');
    const dstArr = dstFile.split('/');
    const srcFileName = srcArr[srcArr.length - 1];
    const dirPath = dstArr.splice(0, dstArr.length - 1).join('/');
    let fileExt : string = '';
    let fileNameWithoutExt = srcFileName;
    if (srcFileName.lastIndexOf('.') !== -1) {
      let tmpValue = srcFileName.split('.').pop();
      if (tmpValue !== undefined) {
        fileExt = tmpValue;
        fileNameWithoutExt = srcFileName.slice(0, srcFileName.length - fileExt.length - 1);
      }
    }
    let newFileName = srcFileName;
    let count = 1;
    while (this.isFileExist(`${dirPath}/${newFileName}`)) {
      if (fileExt === '') {
        newFileName = `${fileNameWithoutExt}(${count})`;
      } else {
        newFileName = `${fileNameWithoutExt}(${count}).${fileExt}`;
      }
      count++;
    }
    try {
      await fs.copyFile(srcFile, `${dirPath}/${newFileName}`);
    } catch (err) {
      console.error(TAG, `moveFile file failed, message = ${err.message}; code = ${err.code}`);
    }
  }
}