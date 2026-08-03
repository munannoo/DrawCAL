#ifndef DRAWCAL_PROGRESS_H
#define DRAWCAL_PROGRESS_H

#include "features/manipulation/Transform.h"

#include <string>

struct GuidedProgressState
{
    int objectType = -1;
    Transform transform = {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f }
    };
    int materialType = 0;
    int attempts = 0;
    bool completed = false;
};

bool InitializeProgressDatabase();
void ShutdownProgressDatabase();
bool IsProgressDatabaseReady();
const std::string& GetProgressDatabasePath();

// Starts or resumes one guided activity and increments its attempt counter.
// On success, restoredState contains the last saved transform/material.
bool BeginGuidedExercise(
    const std::string& exerciseId,
    const GuidedProgressState& defaultState,
    GuidedProgressState& restoredState
);

bool LoadGuidedProgress(
    const std::string& exerciseId,
    GuidedProgressState& state
);

bool SaveGuidedProgress(
    const std::string& exerciseId,
    const GuidedProgressState& state
);

bool MarkGuidedExerciseCompleted(const std::string& exerciseId);

#endif
