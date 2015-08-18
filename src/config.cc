/*
 * Copyright (C) 2014-2015 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file config.cc
/// Implementation of configuration facilities.

#include "config.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "env.h"
#include "error.h"
#include "xml_parser.h"

namespace scram {

Config::Config(const std::string& config_file) : output_path_("") {
  std::ifstream file_stream(config_file.c_str());
  if (!file_stream) {
    throw IOError("The file '" + config_file + "' could not be loaded.");
  }

  std::stringstream stream;
  stream << file_stream.rdbuf();
  file_stream.close();

  std::shared_ptr<XMLParser> parser;
  try {
    parser = std::shared_ptr<XMLParser>(new XMLParser(stream));
    std::stringstream schema;
    std::string schema_path = Env::config_schema();
    std::ifstream schema_stream(schema_path.c_str());
    schema << schema_stream.rdbuf();
    schema_stream.close();
    parser->Validate(schema);
  } catch (ValidationError& err) {
    err.msg("In file '" + config_file + "', " + err.msg());
    throw err;
  }
  const xmlpp::Document* doc = parser->Document();
  const xmlpp::Node* root = doc->get_root_node();
  assert(root->get_name() == "config");
  xmlpp::NodeSet roots_children = root->find("./*");
  xmlpp::NodeSet::iterator it_ch;
  for (it_ch = roots_children.begin();
       it_ch != roots_children.end(); ++it_ch) {
    const xmlpp::Element* element = static_cast<const xmlpp::Element*>(*it_ch);
    std::string name = element->get_name();
    if (name == "input-files") {
      xmlpp::NodeSet input_files = element->find("./*");
      assert(!input_files.empty());
      xmlpp::NodeSet::iterator it_if;
      for (it_if = input_files.begin(); it_if != input_files.end(); ++it_if) {
        const xmlpp::Element* file = static_cast<const xmlpp::Element*>(*it_if);
        assert(file->get_name() == "file");
        input_files_.push_back(file->get_child_text()->get_content());
      }
    } else if (name == "output-path") {
      output_path_ = element->get_child_text()->get_content();

    } else if (name == "options") {
      xmlpp::NodeSet options = element->find("./*");
      xmlpp::NodeSet::iterator it_op;
      for (it_op = options.begin(); it_op != options.end(); ++it_op) {
        const xmlpp::Element* option_group =
            static_cast<const xmlpp::Element*>(*it_op);
        std::string name = option_group->get_name();
        if (name == "analysis") {
          Config::SetAnalysis(option_group);

        } else if (name == "approximations") {
          Config::SetApprox(option_group);

        } else if (name == "limits") {
          Config::SetLimits(option_group);
        }
      }
    }
  }
}

void Config::SetAnalysis(const xmlpp::Element* analysis) {
  const xmlpp::Element::AttributeList attr = analysis->get_attributes();
  xmlpp::Element::AttributeList::const_iterator it;
  for (it = attr.begin(); it != attr.end(); ++it) {
    const xmlpp::Attribute* type = *it;
    std::string name = type->get_name();
    bool flag = Config::GetBoolFromString(type->get_value());
    if (name == "probability") {
      settings_.probability_analysis(flag);

    } else if (name == "importance") {
      settings_.importance_analysis(flag);

    } else if (name == "uncertainty") {
      settings_.uncertainty_analysis(flag);

    } else if (name == "ccf") {
      settings_.ccf_analysis(flag);
    }
  }
}

void Config::SetApprox(const xmlpp::Element* approx) {
  xmlpp::NodeSet elements = approx->find("./*");
  assert(elements.size() == 1);
  xmlpp::NodeSet::iterator it;
  for (it = elements.begin(); it != elements.end(); ++it) {
    std::string name = (*it)->get_name();
    assert(name == "rare-event" || name == "mcub");
    settings_.approx(name);
  }
}

void Config::SetLimits(const xmlpp::Element* limits) {
  xmlpp::NodeSet elements = limits->find("./*");
  xmlpp::NodeSet::iterator it;
  for (it = elements.begin(); it != elements.end(); ++it) {
    const xmlpp::Element* limit = static_cast<const xmlpp::Element*>(*it);
    std::string name = limit->get_name();
    std::string content = limit->get_child_text()->get_content();
    if (name == "limit-order") {
      settings_.limit_order(boost::lexical_cast<int>(content));

    } else if (name == "cut-off") {
      settings_.cut_off(boost::lexical_cast<double>(content));

    } else if (name == "number-of-sums") {
      settings_.num_sums(boost::lexical_cast<int>(content));

    } else if (name == "mission-time") {
      settings_.mission_time(boost::lexical_cast<double>(content));

    } else if (name == "number-of-trials") {
      settings_.num_trials(boost::lexical_cast<int>(content));

    } else if (name == "seed") {
      settings_.seed(boost::lexical_cast<int>(content));
    }
  }
}

bool Config::GetBoolFromString(const std::string& flag) {
  assert(flag == "1" || flag == "true" || flag == "0" || flag == "false");
  if (flag == "1" || flag == "true") return true;
  return false;
}

}  // namespace scram
