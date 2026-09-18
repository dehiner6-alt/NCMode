#include <Geode/Geode.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

using namespace geode::prelude;

// Clase encargada de buscar los reemplazos en los índices de FlafyDev
class SongReplacer {
public:
    static int getReplacement(int originalID) {
        // Leemos el archivo sfh-index.min.json desde los recursos del mod
        auto jsonPath = Mod::get()->getResourcesDir() / "sfh-index.min.json";
        if (!std::filesystem::exists(jsonPath)) return -1;
        
        auto fileContent = utils::file::readString(jsonPath);
        if (!fileContent.isOk()) return -1;
        
        auto jsonRes = matjson::parse(fileContent.unwrap());
        if (!jsonRes.isOk()) return -1;
        
        auto json = jsonRes.unwrap();
        std::string idStr = std::to_string(originalID);
        
        // Verificamos si el ID está en el índice principal
        if (json.contains(idStr)) {
            auto data = json[idStr];
            if (data.isNumber()) {
                return data.asInt();
            } else if (data.isObject() && data.contains("id")) {
                return data["id"].asInt();
            }
        }
        
        // Si no está en SFH, buscamos en el auto-nong-index
        auto nongPath = Mod::get()->getResourcesDir() / "auto-nong-index.min.json";
        if (std::filesystem::exists(nongPath)) {
            auto nongContent = utils::file::readString(nongPath);
            if (nongContent.isOk()) {
                auto nongRes = matjson::parse(nongContent.unwrap());
                if (nongRes.isOk()) {
                    auto nongJson = nongRes.unwrap();
                    if (nongJson.contains(idStr)) {
                        auto nongData = nongJson[idStr];
                        if (nongData.isNumber()) {
                            return nongData.asInt();
                        } else if (nongData.isObject() && nongData.contains("id")) {
                            return nongData["id"].asInt();
                        }
                    }
                }
            }
        }
        
        return -1; // No requiere reemplazo
    }
};

// Gancho para interceptar la carga de información de canciones del juego
class $modify(NoCopyrightMusicManager, MusicDownloadManager) {
    void onGetSongInfoFinished(CCObject* sender) {
        MusicDownloadManager::onGetSongInfoFinished(sender);

        SongInfoObject* songInfo = typeinfo_cast<SongInfoObject*>(sender);
        if (!songInfo) {
            return;
        }

        int originalID = songInfo->m_songID;
        int replacementID = SongReplacer::getReplacement(originalID);
        
        if (replacementID != -1) {
            log::info("[NoCopyrightMod] ¡Canción con copyright detectada! Original: {}, Redirigiendo a: {}", originalID, replacementID);
            
            // Modificamos el ID para redirigirlo a la versión sin copyright procesada por Jukebox
            songInfo->m_songID = replacementID;
        }
    }
};

