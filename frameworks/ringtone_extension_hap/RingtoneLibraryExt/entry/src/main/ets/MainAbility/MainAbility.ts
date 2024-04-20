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

import Ability from '@ohos.app.ability.UIAbility';

export default class MainAbility extends Ability {
  onCreate(want, launchParam): void {
    console.log('[Ringtone] MainAbility onCreate');
    globalThis.abilityWant = want;
  }

  onDestroy(): void {
    console.log('[Ringtone] MainAbility onDestroy');
  }

  onWindowStageCreate(windowStage): void {
    // Main window is created, set main page for this ability
    console.log('[Ringtone] MainAbility onWindowStageCreate');
    windowStage.setUIContent(this.context, 'pages/index', null);
  }

  onWindowStageDestroy(): void {
    // Main window is destroyed, release UI related resources
    console.log('[Ringtone] MainAbility onWindowStageDestroy');
  }

  onForeground(): void {
    // Ability has brought to foreground
    console.log('[Ringtone] MainAbility onForeground');
  }

  onBackground(): void {
    // Ability has back to background
    console.log('[Ringtone] MainAbility onBackground');
  }
};
