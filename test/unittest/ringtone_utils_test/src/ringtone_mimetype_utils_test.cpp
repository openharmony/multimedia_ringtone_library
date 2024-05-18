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

#include "ringtone_mimetype_utils_test.h"

#include <algorithm>

#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_type.h"

using std::string;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const std::map<string, string> g_testExt2MimeType = {
    { "wrf", "video/x-webex" },
    { "mov", "video/quicktime" },
    { "m4v", "video/mp4" },
    { "f4v", "video/mp4" },
    { "mp4v", "video/mp4" },
    { "mpeg4", "video/mp4" },
    { "mp4", "video/mp4" },
    { "m2ts", "video/mp2t" },
    { "mts", "video/mp2t" },
    { "3gpp2", "video/3gpp2" },
    { "3gp2", "video/3gpp2" },
    { "3g2", "video/3gpp2" },
    { "3gpp", "video/3gpp" },
    { "3gp", "video/3gpp" },
    { "vcf", "text/x-vcard" },
    { "cpp", "text/x-c++src" },
    { "c++", "text/x-c++src" },
    { "cxx", "text/x-c++src" },
    { "cc", "text/x-c++src" },
    { "hpp", "text/x-c++hdr" },
    { "h++", "text/x-c++hdr" },
    { "hxx", "text/x-c++hdr" },
    { "hh", "text/x-c++hdr" },
    { "html", "text/html" },
    { "htm", "text/html" },
    { "shtml", "text/html" },
    { "md", "text/markdown" },
    { "markdown", "text/markdown" },
    { "java", "text/x-java" },
    { "py", "text/x-python" },
    { "ts", "video/mp2ts" },
    { "rtf", "text/rtf" },
    { "pef", "image/x-pentax-pef" },
    { "nrw", "image/x-nikon-nrw" },
    { "raf", "image/x-fuji-raf" },
    { "jpg", "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "jpe", "image/jpeg"},
    { "raw", "image/x-dcraw" },
    { "cur", "image/ico" },
    { "heif", "image/heif" },
    { "hif", "image/heif" },
    { "prc", "application/x-mobipocket-ebook" },
    { "mobi", "application/x-mobipocket-ebook" },
    { "bmp", "image/bmp" },
    { "bm", "image/bmp" },
    { "srt", "application/x-subrip" },
    { "phps", "text/text" },
    { "m3u", "text/text" },
    { "m3u8", "text/text" },
    { "css", "text/css" },
    { "webarchivexml", "application/x-webarchive-xml" },
    { "stl", "application/vnd.ms-pki.stl" },
    { "pcf", "application/x-font" },
    { "yt", "application/vnd.youtube.yt" },
    { "imy", "audio/imelody" },
    { "avif", "image/avif" },
    { "vor", "application/vnd.stardivision.writer" },
    { "pot", "application/vnd.ms-powerpoint" },
    { "csv", "text/comma-separated-values" },
    { "webarchive", "application/x-webarchive" },
    { "png", "image/png" },
    { "ttml", "application/ttml+xml" },
    { "dfxp", "application/ttml+xml" },
    { "webp", "image/webp" },
    { "pgp", "application/pgp-signature" },
    { "dng", "image/x-adobe-dng" },
    { "p12", "application/x-pkcs12" },
    { "pfx", "application/x-pkcs12" },
    { "mka", "audio/x-matroska" },
    { "wpl", "application/vnd.ms-wpl" },
    { "webm", "video/webm" },
    { "sdp", "application/sdp" },
    { "ra", "audio/x-pn-realaudio" },
    { "gif", "image/gif" },
    { "smf", "audio/sp-midi" },
    { "ogg", "audio/ogg" },
    { "mp3", "audio/mpeg" },
    { "mp2", "audio/mpeg" },
    { "mp1", "audio/mpeg" },
    { "mpa", "audio/mpeg" },
    { "m4r", "audio/mpeg" },
    { "lrc", "application/lrc" },
    { "crt", "application/x-x509-ca-cert" },
    { "der", "application/x-x509-ca-cert" },
    { "heics", "image/heic-sequence" },
    { "heifs", "image/heic-sequence" },
    { "flac", "audio/flac" },
    { "epub", "application/epub+zip" },
    { "3ga", "audio/3gpp" },
    { "mxmf", "audio/mobile-xmf" },
    { "rss", "application/rss+xml" },
    { "h264", "video/H264" },
    { "heic", "image/heic" },
    { "wav", "audio/wav" },
    { "aac", "audio/aac" },
    { "adts", "audio/aac" },
    { "adt", "audio/aac" },
    { "snd", "audio/basic" },
    { "xml", "text/xml" },
    { "rtttl", "audio/midi" },
    { "xmf", "audio/midi" },
    { "rtx", "audio/midi" },
    { "arw", "image/x-sony-arw" },
    { "ico", "image/x-icon" },
    { "m3u", "audio/mpegurl" },
    { "smil", "application/smil+xml" },
    { "mpeg", "video/mpeg" },
    { "mpeg2", "video/mpeg" },
    { "mpv2", "video/mpeg" },
    { "mp2v", "video/mpeg" },
    { "m2v", "video/mpeg" },
    { "m2t", "video/mpeg" },
    { "mpeg1", "video/mpeg" },
    { "mpv1", "video/mpeg" },
    { "mp1v", "video/mpeg" },
    { "m1v", "video/mpeg" },
    { "mpg", "video/mpeg" },
    { "amr", "audio/amr" },
    { "mkv", "video/x-matroska" },
    { "mp3", "audio/x-mpeg" },
    { "rw2", "image/x-panasonic-rw2" },
    { "svg", "image/svg+xml" },
    { "ac3", "audio/ac3" },
    { "a52", "audio/ac3" },
    { "m4a", "audio/mp4" },
    { "m4b", "audio/mp4" },
    { "m4p", "audio/mp4" },
    { "f4a", "audio/mp4" },
    { "f4b", "audio/mp4" },
    { "f4p", "audio/mp4" },
    { "diff", "text/plain" },
    { "po", "text/plain" },
    { "txt", "text/plain" },
    { "srw", "image/x-samsung-srw" },
    { "pem", "application/x-pem-file" },
    { "cer", "application/pkix-cert" },
    { "json", "application/json" },
    { "js", "application/javascript" },
    { "rar", "application/rar" },
    { "zip", "application/zip" },
    { "pdf", "application/pdf" },
    { "doc", "application/msword" },
    { "xls", "application/ms-excel" },
    { "ppt", "application/vnd.ms-powerpoint" },
    { "docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { "xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { "pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
    {"medialib_GetMimeTypeFromExtension_test", "application/octet-stream"}
};

const std::map<string, RingtoneMediaType> g_testMimeType2MediaType = {
    { "application/epub+zip", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/lrc", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/pkix-cert", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/rss+xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/sdp", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/smil+xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/ttml+xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/vnd.ms-pki.stl", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/vnd.ms-powerpoint", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/vnd.ms-wpl", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/vnd.stardivision.writer", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/vnd.youtube.yt", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-font", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-mobipocket-ebook", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-pem-file", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-pkcs12", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-subrip", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-webarchive", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-webarchive-xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/pgp-signature", RINGTONE_MEDIA_TYPE_INVALID },
    { "application/x-x509-ca-cert", RINGTONE_MEDIA_TYPE_INVALID },
    { "audio/3gpp", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/ac3", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/amr", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/imelody", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/midi", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/mobile-xmf", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/mp4", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/mpegurl", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/sp-midi", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/x-matroska", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/x-pn-realaudio", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/x-mpeg", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/aac", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/basic", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/flac", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/mpeg", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/wav", RINGTONE_MEDIA_TYPE_AUDIO },
    { "audio/ogg", RINGTONE_MEDIA_TYPE_AUDIO },
    { "image/gif", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/heic", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/heic-sequence", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/bmp", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/heif", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/avif", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/ico", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/webp", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-adobe-dng", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-fuji-raf", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-icon", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-nikon-nrw", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-panasonic-rw2", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-pentax-pef", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-samsung-srw", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-sony-arw", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/x-dcraw", RINGTONE_MEDIA_TYPE_INVALID},
    { "image/jpeg", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/png", RINGTONE_MEDIA_TYPE_INVALID },
    { "image/svg+xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/3gpp2", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/3gpp", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/avi", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/mp4", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/mp2t", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/mp2ts", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/vnd.youtube.yt", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/x-webex", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/mpeg", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/quicktime", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/x-matroska", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/webm", RINGTONE_MEDIA_TYPE_INVALID },
    { "video/H264", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/comma-separated-values", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/plain", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/rtf", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/text", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/xml", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/x-vcard", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/x-c++hdr", RINGTONE_MEDIA_TYPE_INVALID },
    { "text/x-c++src", RINGTONE_MEDIA_TYPE_INVALID },
    { "", RINGTONE_MEDIA_TYPE_INVALID }
};

void RingtoneMimeTypeTest::SetUpTestCase() {}

void RingtoneMimeTypeTest::TearDownTestCase() {}
void RingtoneMimeTypeTest::SetUp() {}
void RingtoneMimeTypeTest::TearDown(void) {}

HWTEST_F(RingtoneMimeTypeTest, ringtoneMimeType_InitMimeTypeMap_Test_001, TestSize.Level0)
{
    auto ret = RingtoneMimeTypeUtils::InitMimeTypeMap();
    ASSERT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneMimeTypeTest, ringtoneMimeType_GetMimeTypeFromExtension_Test_001, TestSize.Level0)
{
    auto ret = RingtoneMimeTypeUtils::InitMimeTypeMap();
    ASSERT_EQ(ret, E_OK);
    for (const auto &item : g_testExt2MimeType) {
        auto mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(item.first);
        ASSERT_EQ(mimeType, item.second);
        string upperExtension = item.first;
        std::transform(upperExtension.begin(), upperExtension.end(), upperExtension.begin(), ::toupper);
        mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(upperExtension);
        ASSERT_EQ(mimeType, item.second);
    }
}

HWTEST_F(RingtoneMimeTypeTest, ringtoneMimeType_GetMediaTypeFromMimeType_Test_001, TestSize.Level0)
{
    auto ret = RingtoneMimeTypeUtils::InitMimeTypeMap();
    ASSERT_EQ(ret, E_OK);
    for (const auto &item : g_testMimeType2MediaType) {
        auto mediaType = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(item.first);
        ASSERT_EQ(mediaType, item.second);
    }
}
} // namespace Media
} // namespace OHOS
