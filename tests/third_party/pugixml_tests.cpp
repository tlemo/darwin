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

#include <third_party/pugixml/pugixml.h>

#include <core/utils.h>
#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <string.h>
#include <sstream>
using namespace std;

namespace pugixml_tests {

void visit(const pugi::xml_node& parent, FILE* output, int level = 0) {
  const int indent_size = level * 2;
  fprintf(output, "%*snode '%s' (%s)\n", indent_size, "", parent.name(), parent.value());

  // attributes
  for (const auto& attr : parent.attributes())
    fprintf(
        output, "%*s  attribute '%s'='%s'\n", indent_size, "", attr.name(), attr.value());

  // children
  for (const auto& node : parent)
    visit(node, output, level + 1);
}

TEST(PugiXmlTest, Load) {
  core_test::TestCaseOutput output;

  string xml_filename = string(TEST_INPUT_PATH) + "/perf.xml";

  pugi::xml_document doc;
  auto result = doc.load_file(xml_filename.c_str());
  EXPECT_TRUE(result);

  fprintf(output, "Loading xml document: %s\n", result.description());
  fprintf(output, "---------------------------------------\n");

  visit(doc, output);
}

TEST(PugiXmlTest, Create) {
  core_test::TestCaseOutput output;

  pugi::xml_document doc;

  auto node1 = doc.append_child("child");
  auto attr1 = node1.append_attribute("key");
  attr1.set_value("value");

  // simple node with many attributes
  // (intentionally creating ill-formed xml with duplicates)
  auto node2 = node1.append_child("variable");
  node2.append_attribute("value").set_value("123.0");
  node2.append_attribute("value").set_value("20");
  node2.append_attribute("value2").set_value("aaa");
  node2.append_attribute("value2").set_value(" foo_bar ");
  node2.append_attribute("value2").set_value("0.0");
  node2.append_attribute("value").set_value("1");
  node2.append_attribute("value").set_value("2");
  node2.append_attribute("value").set_value("3");
  node2.append_attribute("value").set_value("4");

  // comment
  auto comment = node2.append_child(pugi::node_comment);
  comment.set_value("haha, i'm just a comment, ignore me    ");

  // node with content
  auto node3 = node1.append_child("content");
  node3.append_child(pugi::node_pcdata).set_value("... this is the content...  ");

  // alternative content creation
  auto node3b = node1.append_child("content");
  node3b.append_attribute("attribute").set_value("value");
  node3b.text().set("1234567890");

  // yet another alternative content creation?
  // (DOES NOT WORK)
  auto node3c = node1.append_child("content");
  node3c.set_value("hehehehe");

  // saving the document (default formatting options)
  stringstream default_format;
  doc.save(default_format);
  fprintf(output, "default format options\n");
  fprintf(output, "----------------------------\n");
  fprintf(output, "%s\n", default_format.str().c_str());

  // saving the document (custom formatting options)
  stringstream custom_format;
  doc.save(custom_format, "  ", pugi::format_indent | pugi::format_indent_attributes);
  fprintf(output, "custom format options\n");
  fprintf(output, "----------------------------\n");
  fprintf(output, "%s\n", custom_format.str().c_str());
}

}  // namespace pugixml_tests
