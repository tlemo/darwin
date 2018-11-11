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

#include <QDialog>

namespace conquest_ui {

namespace Ui {
class NewSandboxDialog;
}

class NewSandboxDialog : public QDialog {
  Q_OBJECT

 public:
  static constexpr const char* kAiPlayer = "AI player";
  static constexpr const char* kHumanPlayer = "Human player";
  static constexpr const char* kHandcraftedPlayer = "Handcrafted";
  static constexpr const char* kRandomPlayer = "Random orders";

 public:
  explicit NewSandboxDialog(QWidget* parent = nullptr);
  ~NewSandboxDialog();

  QString bluePlayerType() const;
  QString redPlayerType() const;
  int bluePlayerGeneration() const;
  int redPlayerGeneration() const;

 private slots:
  void on_blue_player_type_currentIndexChanged(const QString& type);
  void on_red_player_type_currentIndexChanged(const QString& type);

 private:
  Ui::NewSandboxDialog* ui;
};

}  // namespace conquest_ui
