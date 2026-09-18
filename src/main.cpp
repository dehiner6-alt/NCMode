#include <Geode/Geode.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

using namespace geode::prelude;

// Estructura para manejar la lógica de búsqueda en los índices de FlafyDev
class SongReplacer {
public:
    static int getReplacement(int originalID) {
        // Leemos el archivo sfh-index.min.json empaquetado en los recursos
        auto jsonPath = Mod::get()->getResourcesDir() / "sfh-index.min.json";
        
        if (!std::filesystem::exists(jsonPath)) return -1;
        
        auto fileContent = utils::file::readString(jsonPath);
        if (!fileContent.isOk()) return -1;
        
        auto jsonRes = matjson::parse(fileContent.unwrap());
        if (!jsonRes.isOk()) return -1;
        
        auto json = jsonRes.unwrap();
        std::string idStr = std::to_string(originalID);
        
        // Verificamos si el ID de la canción está listado en el JSON
        if (json.contains(idStr)) {
            auto data = json[idStr];
            // Dependiendo del formato exacto del JSON de FlafyDev, extraemos el ID de reemplazo
            if (data.isNumber()) {
                return data.asInt();
            } else if (data.isObject() && data.contains("id")) {
                return data["id"].asInt();
            }
        }
        
        // Si no está en SFH, intentamos revisar el auto-nong-index por si acaso
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

// Gancho a MusicDownloadManager para interceptar las canciones del juego
class $modify(NoCopyrightMusicManager, MusicDownloadManager) {
    
    // Nota: El nombre del método y los argumentos exactos se adaptan al volcado de Geode 5 para la 2.208.
    // Aquí interceptamos cuando el juego procesa o descarga la info de una canción por su ID.
    void onGetSongInfoFinished(CCObject* sender) {
        MusicDownloadManager::onGetSongInfoFinished(sender);

        // Extraemos el ID de la canción de forma segura usando los nodos de la 2.208
        // (Dependiendo de la estructura de CCObject, comúnmente se extrae del objeto de info de la canción)
        SongInfoObject* songInfo = typeinfo_cast<SongInfoObject*>(sender);
        if (!songInfo) {
            return;
        }

        int originalID = songInfo->m_songID;
        
        // Consultamos si nuestra canción está en los índices de reemplazo
        int replacementID = SongReplacer::getReplacement(originalID);
        
        if (replacementID != -1) {
            log::info("[NoCopyrightMod] ¡Canción con copyright detectada! ID Original: {}, Redirigiendo a ID seguro: {}", originalID, replacementID);
            
            // Modificamos el ID de la canción en el objeto para que el juego crea que está pidiendo la versión limpia/piano
            songInfo->m_songID = replacementID;
            
            // Opcional: Aquí Geode se encargará de que Jukebox intercepte este nuevo ID si utiliza 
            // el sistema de redirección de audio compatible.
        }
    }
};

