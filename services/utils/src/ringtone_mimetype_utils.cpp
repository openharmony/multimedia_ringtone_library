/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ringtone_mimetype_utils.h"

#include <fstream>

#include "ringtone_log.h"
#include "ringtone_errno.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace nlohmann;

using MimeTypeMap = unordered_map<string, vector<string>>;

MimeTypeMap RingtoneMimeTypeUtils::mediaJsonMap_;
const string DEFAULT_MIME_TYPE = "application/octet-stream";

static const std::unordered_map<std::string, std::vector<std::string>> RINGTONE_MIME_TYPE_MAP = {
    { "application/epub+zip", { "epub" } },
    { "application/lrc", { "lrc"} },
    { "application/pkix-cert", { "cer" } },
    { "application/rss+xml", { "rss" } },
    { "application/sdp", { "sdp" } },
    { "application/smil+xml", { "smil" } },
    { "application/ttml+xml", { "ttml", "dfxp" } },
    { "application/vnd.ms-pki.stl", { "stl" } },
    { "application/vnd.ms-powerpoint", { "pot", "ppt" } },
    { "application/vnd.ms-wpl", { "wpl" } },
    { "application/vnd.stardivision.writer", { "vor" } },
    { "application/vnd.youtube.yt", { "yt" } },
    { "application/x-font", { "pcf" } },
    { "application/x-mobipocket-ebook", { "prc", "mobi" } },
    { "application/x-pem-file", { "pem" } },
    { "application/x-pkcs12", { "p12", "pfx" } },
    { "application/x-subrip", { "srt" } },
    { "application/x-webarchive", { "webarchive" } },
    { "application/x-webarchive-xml", { "webarchivexml" } },
    { "application/pgp-signature", { "pgp" } },
    { "application/x-x509-ca-cert", { "crt", "der" } },
    { "application/json", { "json" } },
    { "application/javascript", { "js" } },
    { "application/zip", { "zip" } },
    { "application/rar", { "rar" } },
    { "application/pdf", { "pdf" } },
    { "application/msword", { "doc" } },
    { "application/ms-excel", { "xls" } },
    { "application/vnd.openxmlformats-officedocument.wordprocessingml.document", { "docx" } },
    { "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", { "xlsx" } },
    { "application/vnd.openxmlformats-officedocument.presentationml.presentation", { "pptx" } },
    { "audio/3gpp", { "3ga" } },
    { "audio/ac3", { "ac3", "a52"} },
    { "audio/amr", { "amr" } },
    { "audio/imelody", { "imy" } },
    { "audio/midi", { "rtttl", "xmf", "rtx" } },
    { "audio/mobile-xmf", { "mxmf"} },
    { "audio/mp4", { "m4a", "m4b", "m4p", "f4a", "f4b", "f4p" } },
    { "audio/mpegurl", { "m3u" } },
    { "audio/sp-midi", { "smf" } },
    { "audio/x-matroska", { "mka" } },
    { "audio/x-pn-realaudio", { "ra" } },
    { "audio/x-mpeg", { "mp3" } },
    { "audio/ffmpeg", { "ape" } },
    { "audio/aac", { "aac", "adts", "adt" } },
    { "audio/basic", { "snd" } },
    { "audio/flac", { "flac" } },
    { "audio/mpeg", { "mp3", "mp2", "mp1", "mpa", "m4r" } },
    { "audio/wav", { "wav" } },
    { "audio/ogg", { "ogg" } },
    { "image/gif", { "gif"} },
    { "image/heic", { "heic" } },
    { "image/heic-sequence", { "heics", "heifs" } },
    { "image/bmp", { "bmp", "bm" } },
    { "image/heif", { "heif", "hif" } },
    { "image/avif", { "avif" } },
    { "image/ico", { "cur" } },
    { "image/webp", { "webp"} },
    { "image/x-adobe-dng", { "dng" } },
    { "image/x-fuji-raf", { "raf" } },
    { "image/x-icon", { "ico" } },
    { "image/x-nikon-nrw", { "nrw" } },
    { "image/x-panasonic-rw2", { "rw2" } },
    { "image/x-pentax-pef", { "pef" } },
    { "image/x-samsung-srw", { "srw" } },
    { "image/x-sony-arw", { "arw" } },
    { "image/jpeg", { "jpg", "jpeg", "jpe" } },
    { "image/png", { "png" } },
    { "image/svg+xml", { "svg" } },
    { "image/x-dcraw", { "raw" } },
    { "video/mp4", { "mp4" } },
    { "text/comma-separated-values", { "csv" } },
    { "text/plain", { "diff", "po", "txt" } },
    { "text/rtf", { "rtf" } },
    { "text/text", { "phps", "m3u", "m3u8" } },
    { "text/xml", { "xml" } },
    { "text/x-vcard", { "vcf" } },
    { "text/x-c++hdr", { "hpp", "h++", "hxx", "hh" } },
    { "text/x-c++src", { "cpp", "c++", "cxx", "cc" } },
    { "text/css", { "css" } },
    { "text/html", { "html", "htm", "shtml"} },
    { "text/markdown", { "md", "markdown" } },
    { "text/x-java", { "java" } },
    { "text/x-python", { "py" } }
};

int32_t RingtoneMimeTypeUtils::InitMimeTypeMap()
{
    RINGTONE_INFO_LOG("InitMimeTypeMap start.");
    mediaJsonMap_ = RINGTONE_MIME_TYPE_MAP;
    if (mediaJsonMap_.empty()) {
        RINGTONE_ERR_LOG("JsonMap is empty");
        return E_FAIL;
    }
    return E_OK;
}

string RingtoneMimeTypeUtils::GetMimeTypeFromExtension(const string &extension)
{
    return GetMimeTypeFromExtension(extension, mediaJsonMap_);
}

string RingtoneMimeTypeUtils::GetMimeTypeFromExtension(const string &extension,
    const MimeTypeMap &mimeTypeMap)
{
    string tmp = extension;
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    for (auto &item : mimeTypeMap) {
        for (auto &ext : item.second) {
            if (ext == tmp) {
                return item.first;
            }
        }
    }
    return DEFAULT_MIME_TYPE;
}

RingtoneMediaType RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(const string &mimeType)
{
    size_t pos = mimeType.find_first_of("/");
    if (pos == string::npos) {
        RINGTONE_ERR_LOG("Invalid mime type: %{public}s", mimeType.c_str());
        return RINGTONE_MEDIA_TYPE_INVALID;
    }
    string prefix = mimeType.substr(0, pos);
    if (prefix == "audio") {
        return RINGTONE_MEDIA_TYPE_AUDIO;
    } else if (prefix == "video") {
        return RINGTONE_MEDIA_TYPE_VIDEO;
    } else {
        RINGTONE_WARN_LOG("Invalid mime type: %{public}s", mimeType.c_str());
        return RINGTONE_MEDIA_TYPE_INVALID;
    }
}
} // namespace Media
} // namespace OHOS
