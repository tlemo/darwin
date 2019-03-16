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

#include <core/darwin.h>
#include <core/tournament_implementations.h>
#include <core/utils.h>

#include <third_party/gtest/gtest.h>

#include <algorithm>
#include <random>
#include <vector>
using namespace std;

namespace tournament_tests {

enum class TestPlayerStrategy {
  AlwaysWin,
  AlwaysLose,
  Draw,           // unless facing an AlwaysX player
  RandomOutcome,  // random outcome (when facing another RandomOutcome)
};

struct TestGenotype : public darwin::Genotype {
  // the hardcoded gameplay strategy
  TestPlayerStrategy strategy = TestPlayerStrategy::RandomOutcome;

  // keep track of the genotypes which participated in the tournament
  mutable bool participated = false;

  unique_ptr<darwin::Brain> grow() const override { FATAL("Not implemented"); }
  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }
  json save() const override { FATAL("Not implemented"); }
  void load(const json&) override { FATAL("Not implemented"); }
};

struct TestPopulation : public darwin::Population {
  size_t size() const override { return genotypes_.size(); }

  darwin::Genotype* genotype(size_t i) override { return &genotypes_[i]; }
  const darwin::Genotype* genotype(size_t i) const override { return &genotypes_[i]; }

  void createPrimordialGeneration(int population_size) override {
    ASSERT_GE(population_size, 2);
    genotypes_.resize(population_size);
  }

  int generation() const override { FATAL("Not implemented"); }
  vector<size_t> rankingIndex() const override { FATAL("Not implemented"); }
  void createNextGeneration() override { FATAL("Not implemented"); }

  void generateTestStrategies() {
    for (size_t i = 0; i < genotypes_.size(); ++i) {
      switch (i) {
        case 0:
          genotypes_[i].strategy = TestPlayerStrategy::AlwaysWin;
          break;
        case 1:
          genotypes_[i].strategy = TestPlayerStrategy::AlwaysLose;
          break;
        case 2:
          genotypes_[i].strategy = TestPlayerStrategy::Draw;
          break;
        default:
          genotypes_[i].strategy = TestPlayerStrategy::RandomOutcome;
          break;
      }
    }

    random_device rd;
    default_random_engine rnd(rd());
    std::shuffle(genotypes_.begin(), genotypes_.end(), rnd);
  }

  void validateResults() {
    // make sure everyone participated in the tournament
    for (const auto& genotype : genotypes_) {
      EXPECT_TRUE(genotype.participated);
    }

    // rank the genotypes
    std::sort(genotypes_.begin(),
              genotypes_.end(),
              [](const TestGenotype& a, const TestGenotype& b) {
                return a.fitness > b.fitness;
              });

    // make sure that AlwaysWin is at the top
    // (potentially tied for the top fitness since we're not guaranteed that
    // the tournament if a full round-robin)
    const auto max_fitness = genotypes_.front().fitness;
    for (auto it = genotypes_.begin(); it != genotypes_.end(); ++it) {
      EXPECT_EQ(it->fitness, max_fitness);
      if (it->strategy == TestPlayerStrategy::AlwaysWin) {
        break;
      }
    }

    // ditto for AlwaysLose
    const auto min_fitness = genotypes_.back().fitness;
    for (auto it = genotypes_.rbegin(); it != genotypes_.rend(); ++it) {
      EXPECT_EQ(it->fitness, min_fitness);
      if (it->strategy == TestPlayerStrategy::AlwaysLose) {
        break;
      }
    }
  }

 private:
  vector<TestGenotype> genotypes_;
};

class TestGameRules : public tournament::GameRules {
 public:
  tournament::Scores scores(tournament::GameOutcome outcome) const override {
    switch (outcome) {
      case tournament::GameOutcome::FirstPlayerWins:
        return { +1, 0 };
      case tournament::GameOutcome::Draw:
        return { +0.4f, +0.4f };
      case tournament::GameOutcome::SecondPlayerWins:
        return { 0, +1 };
      default:
        FATAL("unexpected outcome");
    }
  }

  tournament::GameOutcome play(const darwin::Genotype* player1_genotype,
                               const darwin::Genotype* player2_genotype) const override {
    EXPECT_NE(player1_genotype, player2_genotype);
    auto p1 = dynamic_cast<const TestGenotype*>(player1_genotype);
    auto p2 = dynamic_cast<const TestGenotype*>(player2_genotype);
    p1->participated = true;
    p2->participated = true;

    if (p1->strategy == TestPlayerStrategy::AlwaysWin) {
      EXPECT_NE(p2->strategy, TestPlayerStrategy::AlwaysWin);
      return tournament::GameOutcome::FirstPlayerWins;
    } else if (p2->strategy == TestPlayerStrategy::AlwaysWin) {
      EXPECT_NE(p1->strategy, TestPlayerStrategy::AlwaysWin);
      return tournament::GameOutcome::SecondPlayerWins;
    } else if (p1->strategy == TestPlayerStrategy::AlwaysLose) {
      EXPECT_NE(p2->strategy, TestPlayerStrategy::AlwaysLose);
      return tournament::GameOutcome::SecondPlayerWins;
    } else if (p2->strategy == TestPlayerStrategy::AlwaysLose) {
      EXPECT_NE(p1->strategy, TestPlayerStrategy::AlwaysLose);
      return tournament::GameOutcome::FirstPlayerWins;
    } else if (p1->strategy == TestPlayerStrategy::Draw) {
      return tournament::GameOutcome::Draw;
    } else if (p2->strategy == TestPlayerStrategy::Draw) {
      return tournament::GameOutcome::Draw;
    } else {
      random_device rd;
      default_random_engine rnd(rd());
      uniform_int_distribution<int> outcome_dist(0, 2);
      switch (outcome_dist(rnd)) {
        case 0:
          return tournament::GameOutcome::FirstPlayerWins;
        case 1:
          return tournament::GameOutcome::SecondPlayerWins;
        case 2:
          return tournament::GameOutcome::Draw;
        default:
          FATAL("Unexpected outcome");
      }
    }
  }
};

struct TournamentTest : public testing::TestWithParam<int> {
  void testTournament(tournament::Tournament* tournament) {
    constexpr int kTournamentsCount = 5;

    TestPopulation population;
    TestGameRules rules;

    const int population_size = GetParam();
    population.createPrimordialGeneration(population_size);

    for (int i = 0; i < kTournamentsCount; ++i) {
      population.generateTestStrategies();
      tournament->evaluatePopulation(&population, &rules);
      population.validateResults();
    }
  }
};

TEST_P(TournamentTest, SimpleTournament_Basic) {
  tournament::SimpleTournamentConfig config;
  config.eval_games = 1;
  config.rematches = false;

  tournament::SimpleTournament tournament(config);
  testTournament(&tournament);
}

TEST_P(TournamentTest, SimpleTournament_Full) {
  tournament::SimpleTournamentConfig config;
  config.eval_games = 4;
  config.rematches = true;

  tournament::SimpleTournament tournament(config);
  testTournament(&tournament);
}

TEST_P(TournamentTest, SwissTournament_Basic) {
  tournament::SwissTournamentConfig config;
  config.rounds = 1;
  config.rematches = false;

  tournament::SwissTournament tournament(config);
  testTournament(&tournament);
}

TEST_P(TournamentTest, SwissTournament_Full) {
  tournament::SwissTournamentConfig config;
  config.rounds = 10;
  config.rematches = true;

  tournament::SwissTournament tournament(config);
  testTournament(&tournament);
}

// instantiate the test cases with various population sizes
// (must be even values - some of the tournament implementations require it)
INSTANTIATE_TEST_CASE_P(All, TournamentTest, testing::Values(2, 4, 100));

}  // namespace tournament_tests
