#pragma once

const char* FullPath(const char* file);
const char* FullWritablePath(const char* file);

void LoadProgress();
void SaveProgress();

#ifdef __EMSCRIPTEN__
void SyncPersistentStorage();
#endif
