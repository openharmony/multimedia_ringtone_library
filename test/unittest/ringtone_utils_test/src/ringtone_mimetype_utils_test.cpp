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

using std::string;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const std::map<string, string> g_testExt2MimeType = {
    { "smf", "audio/sp-midi" },
    { "ogg", "audio/ogg" },
    { "mp2", "audio/mpeg" },
    { "mp1", "audio/mpeg" },
    { "mpa", "audio/mpeg" },
    { "m4r", "audio/mpeg" }
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
    { "video/3gpp2", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/3gpp", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/mp4", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/mp2t", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/mp2ts", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/vnd.youtube.yt", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/x-webex", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/mpeg", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/quicktime", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/x-matroska", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/webm", RINGTONE_MEDIA_TYPE_VIDEO },
    { "video/H264", RINGTONE_MEDIA_TYPE_VIDEO },
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
        if (item.first == "yt") {
            ASSERT_EQ(mimeType == "application/vnd.youtube.yt" || mimeType == "video/vnd.youtube.yt", true);
        } else {
            ASSERT_EQ(mimeType, item.second);
        }
        string upperExtension = item.first;
        std::transform(upperExtension.begin(), upperExtension.end(), upperExtension.begin(), ::toupper);
        mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(upperExtension);
        if (item.first == "yt") {
            ASSERT_EQ(mimeType == "application/vnd.youtube.yt" || mimeType == "video/vnd.youtube.yt", true);
        } else {
            ASSERT_EQ(mimeType, item.second);
        }
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
