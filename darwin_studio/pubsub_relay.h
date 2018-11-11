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

#include "console_buffer.h"

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>

#include <QObject>
#include <QString>

#include <stdint.h>
#include <string>

// Maps core::PubSub<T> subscriptions to QT signals
class PubSubRelay : public QObject {
  Q_OBJECT

 private:
  PubSubRelay() {
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<darwin::GenerationSummary>("darwin::GenerationSummary");
    qRegisterMetaType<darwin::EvolutionStage>("darwin::EvolutionStage");

    darwin::evolution()->events.subscribe(
        [this](uint32_t hints) { emit sigEvents(hints); });

    ConsoleBuffer::instance()->console_output.subscribe(
        [this](const std::string& message) {
          emit sigConsoleOutput(QString::fromStdString(message));
        });

    darwin::evolution()->generation_summary.subscribe(
        [this](const darwin::GenerationSummary& summary) {
          emit sigGenerationSummary(summary);
        });

    darwin::evolution()->top_stages.subscribe(
        [this](const darwin::EvolutionStage& stage) { emit sigTopStage(stage); });
  }

 public:
  static PubSubRelay* instance() {
    static PubSubRelay* instance = new PubSubRelay;
    return instance;
  }

 signals:
  void sigEvents(uint32_t hints);
  void sigConsoleOutput(const QString& output);
  void sigGenerationSummary(const darwin::GenerationSummary& summary);
  void sigTopStage(const darwin::EvolutionStage& stage);
};
