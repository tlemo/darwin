// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include <core/stringify.h>
#include <core/tournament.h>
#include <core/simple_tournament.h>
#include <core/swiss_tournament.h>

#include <memory>
using namespace std;

namespace tournament {

//! Tournament type
enum class TournamentType {
  Simple,   //!< A basic tournament implementation
  Swiss,    //!< Swiss-system tournament
};

inline auto customStringify(core::TypeTag<TournamentType>) {
  static auto stringify = new core::StringifyKnownValues<TournamentType>{
    { TournamentType::Simple, "simple" },
    { TournamentType::Swiss, "swiss" },
  };
  return stringify;
}

//! Tournament configurations
struct TournamentVariant : public core::PropertySetVariant<TournamentType> {
  CASE(TournamentType::Simple, simple_tournament, SimpleTournamentConfig);
  CASE(TournamentType::Swiss, swiss_tournament, SwissTournamentConfig);
};

//! Concrete tournament factory
inline unique_ptr<Tournament> create(const TournamentVariant& variant) {
  switch (variant.tag()) {
    case TournamentType::Simple:
      return make_unique<SimpleTournament>(variant.simple_tournament);
    case TournamentType::Swiss:
      return make_unique<SwissTournament>(variant.swiss_tournament);
    default:
      FATAL("Unexpected tournament type");
  }
}

}  // namespace tournament
