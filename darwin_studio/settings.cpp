// Copyright 2018 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "settings.h"

#include <core/logging.h>

#include <third_party/json/json.h>
using nlohmann::json;

#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

Settings g_settings;

constexpr char kSettingsFile[] = "studio_settings.json";

void Settings::init() {
  if (fs::is_regular_file(kSettingsFile)) {
    load();
  } else {
    core::log("Settings not found, using the defaults.\n");
    save();
  }
}

void Settings::load() {
  ifstream settings_file(kSettingsFile);
  CHECK(settings_file, "can't open the settings file");

  try {
    json settings_json;
    settings_file >> settings_json;
    g_settings.fromJson(settings_json);
  } catch (const std::exception& e) {
    core::log("Can't load settings: %s\n", e.what());
    g_settings.resetToDefaultValues();
  }
}

void Settings::save() {
  ofstream settings_file(kSettingsFile);
  CHECK(settings_file, "can't save the settings file");

  settings_file << std::setw(2) << g_settings.toJson();
}
