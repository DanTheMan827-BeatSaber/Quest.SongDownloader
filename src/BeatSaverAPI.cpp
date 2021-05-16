#include "BeatSaverAPI.hpp"

#include "CustomLogger.hpp"

#include "Utils/WebUtils.hpp"
#include "Utils/FileUtils.hpp"

#include "zip.h"

#include "songloader/shared/API.hpp"

#define BASE_URL std::string("https://beatsaver.com")

#define FILE_DOWNLOAD_TIMEOUT 64

namespace BeatSaver::API {

    std::optional<BeatSaver::Beatmap> GetBeatmapByKey(std::string key) {
        auto json = WebUtils::GetJSON(BASE_URL + "/api/maps/detail/" + key);
        if(!json.has_value())
            return std::nullopt;
        BeatSaver::Beatmap beatmap;
        beatmap.Deserialize(json.value().GetObject());
        return beatmap;
    }

    std::optional<BeatSaver::Beatmap> GetBeatmapByHash(std::string hash) {
        auto json = WebUtils::GetJSON(BASE_URL + "/api/maps/by-hash/" + hash);
        if(!json.has_value())
            return std::nullopt;
        BeatSaver::Beatmap beatmap;
        beatmap.Deserialize(json.value().GetObject());
        return beatmap;
    }

    std::optional<BeatSaver::Page> SearchPaged(std::string query, int pageIndex) {
        auto json = WebUtils::GetJSON(BASE_URL + "/api/search/text/" + std::to_string(pageIndex) + "?q=" + query);
        if(!json.has_value())
            return std::nullopt;
        BeatSaver::Page page;
        page.Deserialize(json.value().GetObject());
        return page;
    }

    bool DownloadBeatmap(const BeatSaver::Beatmap& beatmap) {
        auto targetFolder = RuntimeSongLoader::API::GetCustomLevelsPath() + beatmap.GetKey() + " ()[]{}%&.:,;=!-_ (" + beatmap.GetMetadata().GetSongName() + " - " + beatmap.GetMetadata().GetLevelAuthorName() + ")";
        std::string data;
        WebUtils::Get(BASE_URL + beatmap.GetDownloadURL(), FILE_DOWNLOAD_TIMEOUT, data);
        int args = 2;
        int statusCode = zip_stream_extract(data.data(), data.length(), targetFolder.c_str(), +[](const char *name, void *arg) -> int {
            return 0;
        }, &args);
        return statusCode;
    }

    std::vector<uint8_t> GetCoverImage(const BeatSaver::Beatmap& beatmap) {
        std::string data;
        WebUtils::Get(BASE_URL + beatmap.GetCoverURL(), FILE_DOWNLOAD_TIMEOUT, data);
        std::vector<uint8_t> bytes(data.begin(), data.end());
        return bytes;
    }
    

    void GetBeatmapByKeyAsync(std::string key, std::function<void(std::optional<BeatSaver::Beatmap>)> finished) {
        WebUtils::GetJSONAsync(BASE_URL + "/api/maps/detail/" + key,
            [finished] (long httpCode, bool error, rapidjson::Document& document) {
                if(error) {
                    finished(std::nullopt);
                } else {
                    BeatSaver::Beatmap beatmap;
                    beatmap.Deserialize(document.GetObject());
                    finished(beatmap);
                }
            }
        );
    }

    void GetBeatmapByHashAsync(std::string hash, std::function<void(std::optional<BeatSaver::Beatmap>)> finished) {
        WebUtils::GetJSONAsync(BASE_URL + "/api/maps/by-hash/" + hash,
            [finished] (long httpCode, bool error, rapidjson::Document& document) {
                if(error) {
                    finished(std::nullopt);
                } else {
                    BeatSaver::Beatmap beatmap;
                    beatmap.Deserialize(document.GetObject());
                    finished(beatmap);
                }
            }
        );
    }

    void SearchPagedAsync(std::string query, int pageIndex, std::function<void(std::optional<BeatSaver::Page>)> finished) {
        WebUtils::GetJSONAsync(BASE_URL + "/api/search/text/" + std::to_string(pageIndex) + "?q=" + query,
            [finished] (long httpCode, bool error, rapidjson::Document& document) {
                if(error) {
                    finished(std::nullopt);
                } else {
                    BeatSaver::Page page;
                    page.Deserialize(document.GetObject());
                    finished(page);
                }
            }
        );
    }

    void DownloadBeatmapAsync(const BeatSaver::Beatmap& beatmap, std::function<void(bool)> finished, std::function<void(float)> progressUpdate) {
        WebUtils::GetAsync(BASE_URL + beatmap.GetDownloadURL(), FILE_DOWNLOAD_TIMEOUT,
            [beatmap, finished] (long httpCode, std::string data) {
                auto targetFolder = RuntimeSongLoader::API::GetCustomLevelsPath() + FileUtils::FixIlegalName(beatmap.GetKey() + " (" + beatmap.GetMetadata().GetSongName() + " - " + beatmap.GetMetadata().GetLevelAuthorName() + ")");
                int args = 2;
                int statusCode = zip_stream_extract(data.data(), data.length(), targetFolder.c_str(), +[](const char *name, void *arg) -> int {
                    return 0;
                }, &args);
                finished(statusCode);
            }, progressUpdate
        );
    }

    void GetCoverImageAsync(const BeatSaver::Beatmap& beatmap, std::function<void(std::vector<uint8_t>)> finished, std::function<void(float)> progressUpdate) {
        WebUtils::GetAsync(BASE_URL + beatmap.GetCoverURL(), FILE_DOWNLOAD_TIMEOUT,
            [beatmap, finished] (long httpCode, std::string data) {
                std::vector<uint8_t> bytes(data.begin(), data.end());
                finished(bytes);
            }, progressUpdate
        );
    }

}