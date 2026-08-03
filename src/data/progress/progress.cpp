#include "data/progress/progress.h"

#include "sqlite3.h"
#include "raylib.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace
{
    sqlite3* progressDatabase = nullptr;
    std::string progressDatabasePath;

    bool ExecuteSql(const char* sql)
    {
        char* errorMessage = nullptr;
        const int result = sqlite3_exec(
            progressDatabase,
            sql,
            nullptr,
            nullptr,
            &errorMessage
        );

        if (result == SQLITE_OK) return true;

        TraceLog(
            LOG_ERROR,
            "Progress database error: %s",
            errorMessage != nullptr ? errorMessage : sqlite3_errmsg(progressDatabase)
        );
        sqlite3_free(errorMessage);
        return false;
    }

    std::filesystem::path ResolveProgressDirectory()
    {
#if defined(_WIN32)
        if (const char* localAppData = std::getenv("LOCALAPPDATA"))
            return std::filesystem::path(localAppData) / "DrawCAL";
#else
        if (const char* xdgDataHome = std::getenv("XDG_DATA_HOME"))
            return std::filesystem::path(xdgDataHome) / "DrawCAL";
        if (const char* userHome = std::getenv("HOME"))
            return std::filesystem::path(userHome) / ".local" / "share" / "DrawCAL";
#endif
        return std::filesystem::current_path() / "DrawCALData";
    }

    void BindState(sqlite3_stmt* statement, const std::string& exerciseId, const GuidedProgressState& state)
    {
        sqlite3_bind_text(statement, 1, exerciseId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, state.objectType);
        sqlite3_bind_double(statement, 3, state.transform.translation.x);
        sqlite3_bind_double(statement, 4, state.transform.translation.y);
        sqlite3_bind_double(statement, 5, state.transform.translation.z);
        sqlite3_bind_double(statement, 6, state.transform.rotation.x);
        sqlite3_bind_double(statement, 7, state.transform.rotation.y);
        sqlite3_bind_double(statement, 8, state.transform.rotation.z);
        sqlite3_bind_double(statement, 9, state.transform.rotation.w);
        sqlite3_bind_double(statement, 10, state.transform.scale.x);
        sqlite3_bind_double(statement, 11, state.transform.scale.y);
        sqlite3_bind_double(statement, 12, state.transform.scale.z);
        sqlite3_bind_int(statement, 13, state.materialType);
    }
}

bool InitializeProgressDatabase()
{
    if (progressDatabase != nullptr) return true;

    const std::filesystem::path directory = ResolveProgressDirectory();
    std::error_code directoryError;
    std::filesystem::create_directories(directory, directoryError);
    if (directoryError)
    {
        TraceLog(LOG_ERROR, "Could not create DrawCAL data directory: %s", directoryError.message().c_str());
        return false;
    }

    progressDatabasePath = (directory / "drawcal.db").u8string();
    const int openResult = sqlite3_open_v2(
        progressDatabasePath.c_str(),
        &progressDatabase,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        nullptr
    );

    if (openResult != SQLITE_OK)
    {
        TraceLog(
            LOG_ERROR,
            "Could not open progress database: %s",
            progressDatabase != nullptr ? sqlite3_errmsg(progressDatabase) : "unknown SQLite error"
        );
        ShutdownProgressDatabase();
        return false;
    }

    sqlite3_busy_timeout(progressDatabase, 2000);

    if (!ExecuteSql("PRAGMA journal_mode = WAL;") ||
        !ExecuteSql(
            "CREATE TABLE IF NOT EXISTS guided_progress ("
            "exercise_id TEXT PRIMARY KEY NOT NULL,"
            "object_type INTEGER NOT NULL,"
            "position_x REAL NOT NULL DEFAULT 0,"
            "position_y REAL NOT NULL DEFAULT 0,"
            "position_z REAL NOT NULL DEFAULT 0,"
            "rotation_x REAL NOT NULL DEFAULT 0,"
            "rotation_y REAL NOT NULL DEFAULT 0,"
            "rotation_z REAL NOT NULL DEFAULT 0,"
            "rotation_w REAL NOT NULL DEFAULT 1,"
            "scale_x REAL NOT NULL DEFAULT 1,"
            "scale_y REAL NOT NULL DEFAULT 1,"
            "scale_z REAL NOT NULL DEFAULT 1,"
            "material_type INTEGER NOT NULL DEFAULT 0,"
            "attempts INTEGER NOT NULL DEFAULT 0,"
            "completed INTEGER NOT NULL DEFAULT 0 CHECK(completed IN (0, 1)),"
            "last_opened_at TEXT,"
            "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ");"
        ) ||
        !ExecuteSql("PRAGMA user_version = 1;"))
    {
        ShutdownProgressDatabase();
        return false;
    }

    TraceLog(LOG_INFO, "Progress database ready: %s", progressDatabasePath.c_str());
    return true;
}

void ShutdownProgressDatabase()
{
    if (progressDatabase != nullptr)
    {
        sqlite3_close(progressDatabase);
        progressDatabase = nullptr;
    }
}

bool IsProgressDatabaseReady()
{
    return progressDatabase != nullptr;
}

const std::string& GetProgressDatabasePath()
{
    return progressDatabasePath;
}

bool LoadGuidedProgress(const std::string& exerciseId, GuidedProgressState& state)
{
    if (progressDatabase == nullptr) return false;

    static constexpr const char* sql =
        "SELECT object_type, position_x, position_y, position_z,"
        "rotation_x, rotation_y, rotation_z, rotation_w,"
        "scale_x, scale_y, scale_z, material_type, attempts, completed "
        "FROM guided_progress WHERE exercise_id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(progressDatabase, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        TraceLog(LOG_ERROR, "Could not prepare progress load: %s", sqlite3_errmsg(progressDatabase));
        return false;
    }

    sqlite3_bind_text(statement, 1, exerciseId.c_str(), -1, SQLITE_TRANSIENT);
    const int stepResult = sqlite3_step(statement);

    if (stepResult == SQLITE_ROW)
    {
        state.objectType = sqlite3_column_int(statement, 0);
        state.transform.translation = {
            static_cast<float>(sqlite3_column_double(statement, 1)),
            static_cast<float>(sqlite3_column_double(statement, 2)),
            static_cast<float>(sqlite3_column_double(statement, 3))
        };
        state.transform.rotation = {
            static_cast<float>(sqlite3_column_double(statement, 4)),
            static_cast<float>(sqlite3_column_double(statement, 5)),
            static_cast<float>(sqlite3_column_double(statement, 6)),
            static_cast<float>(sqlite3_column_double(statement, 7))
        };
        state.transform.scale = {
            static_cast<float>(sqlite3_column_double(statement, 8)),
            static_cast<float>(sqlite3_column_double(statement, 9)),
            static_cast<float>(sqlite3_column_double(statement, 10))
        };
        state.materialType = sqlite3_column_int(statement, 11);
        state.attempts = sqlite3_column_int(statement, 12);
        state.completed = sqlite3_column_int(statement, 13) != 0;
        sqlite3_finalize(statement);
        return true;
    }

    if (stepResult != SQLITE_DONE)
        TraceLog(LOG_ERROR, "Could not load progress: %s", sqlite3_errmsg(progressDatabase));

    sqlite3_finalize(statement);
    return false;
}

bool BeginGuidedExercise(
    const std::string& exerciseId,
    const GuidedProgressState& defaultState,
    GuidedProgressState& restoredState)
{
    if (progressDatabase == nullptr) return false;

    static constexpr const char* sql =
        "INSERT INTO guided_progress ("
        "exercise_id, object_type, position_x, position_y, position_z,"
        "rotation_x, rotation_y, rotation_z, rotation_w,"
        "scale_x, scale_y, scale_z, material_type, attempts, last_opened_at"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1, CURRENT_TIMESTAMP) "
        "ON CONFLICT(exercise_id) DO UPDATE SET "
        "attempts = guided_progress.attempts + 1,"
        "last_opened_at = CURRENT_TIMESTAMP;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(progressDatabase, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        TraceLog(LOG_ERROR, "Could not prepare guided progress start: %s", sqlite3_errmsg(progressDatabase));
        return false;
    }

    BindState(statement, exerciseId, defaultState);
    const bool saved = sqlite3_step(statement) == SQLITE_DONE;
    if (!saved)
        TraceLog(LOG_ERROR, "Could not start guided exercise: %s", sqlite3_errmsg(progressDatabase));
    sqlite3_finalize(statement);

    if (!saved) return false;
    return LoadGuidedProgress(exerciseId, restoredState);
}

bool SaveGuidedProgress(const std::string& exerciseId, const GuidedProgressState& state)
{
    if (progressDatabase == nullptr) return false;

    static constexpr const char* sql =
        "INSERT INTO guided_progress ("
        "exercise_id, object_type, position_x, position_y, position_z,"
        "rotation_x, rotation_y, rotation_z, rotation_w,"
        "scale_x, scale_y, scale_z, material_type, attempts"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1) "
        "ON CONFLICT(exercise_id) DO UPDATE SET "
        "object_type = excluded.object_type,"
        "position_x = excluded.position_x, position_y = excluded.position_y, position_z = excluded.position_z,"
        "rotation_x = excluded.rotation_x, rotation_y = excluded.rotation_y,"
        "rotation_z = excluded.rotation_z, rotation_w = excluded.rotation_w,"
        "scale_x = excluded.scale_x, scale_y = excluded.scale_y, scale_z = excluded.scale_z,"
        "material_type = excluded.material_type, updated_at = CURRENT_TIMESTAMP;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(progressDatabase, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        TraceLog(LOG_ERROR, "Could not prepare progress save: %s", sqlite3_errmsg(progressDatabase));
        return false;
    }

    BindState(statement, exerciseId, state);
    const bool saved = sqlite3_step(statement) == SQLITE_DONE;
    if (!saved)
        TraceLog(LOG_ERROR, "Could not save guided progress: %s", sqlite3_errmsg(progressDatabase));
    sqlite3_finalize(statement);
    return saved;
}

bool MarkGuidedExerciseCompleted(const std::string& exerciseId)
{
    if (progressDatabase == nullptr) return false;

    static constexpr const char* sql =
        "UPDATE guided_progress SET completed = 1, updated_at = CURRENT_TIMESTAMP "
        "WHERE exercise_id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(progressDatabase, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(statement, 1, exerciseId.c_str(), -1, SQLITE_TRANSIENT);
    const bool saved = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return saved;
}
