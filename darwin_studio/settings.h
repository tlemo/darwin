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

#pragma once

#include <core/properties.h>

// TODO:
// - split into settings / persistent internal state
// - recent N universes?
struct Settings : public core::PropertySet {
  PROPERTY(last_universe, string, "", "Last Darwin universe");

  PROPERTY(last_directory, string, "", "Last universe location");

  PROPERTY(reopen_last_universe,
           bool,
           true,
           "Automatically reopen the last universe at startup");

  PROPERTY(auto_save_ui_layout, bool, true, "Automatically save the UI layout on exit");

  PROPERTY(spline_fitness_series, bool, false, "Use spline for the fitness chart series");

  PROPERTY(qt_base_layout_data,
           string,
           "",
           "Persists main window's layout (no experiment)");

  PROPERTY(qt_experiment_layout_data,
           string,
           "",
           "Persists main window's layout (experiment)");

  PROPERTY(qt_geometry_data, string, "", "Persists main window's geometry");

  // load the persistent settings
  // (and create them if not already present on disk)
  void init();

  void load();
  void save();
};

extern Settings g_settings;
