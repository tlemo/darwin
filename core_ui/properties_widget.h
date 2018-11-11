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

#include <QString>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <string>
using namespace std;

namespace core_ui {

class PropertyItem;
class BoundPropertyItem;
class PropertiesWidget;

//! Base class for all the items in a PropertiesWidget
class PropertyItemBase : public QTreeWidgetItem {
 public:
  //! Parent widget
  PropertiesWidget* parentWidget() const;
  
  //! Parent item (or `nullptr` for top level items)
  PropertyItemBase* parent() const;
  
  //! Returns `true` if the item's value has changed,
  //! or if there are any modified sub-items
  virtual bool isModified() const;

  //! Notify the item that it's value (or a sub-item) have been changed
  void valueChanged();
  
 protected:
  explicit PropertyItemBase(PropertyItemBase* parent) : QTreeWidgetItem(parent) {}
};

//! Grouping section for a set of related sub-items
class PropertiesSectionItem : public PropertyItemBase {
  friend class PropertiesWidget;

 public:
  //! Creates a new property item
  PropertyItem* addProperty(const string& name);
  
  //! Creates a property item bound to a core::Property
  BoundPropertyItem* addProperty(core::Property* property);

 private:
  explicit PropertiesSectionItem(const string& name);
};

//! A basic, manually-updated property item
class PropertyItem : public PropertyItemBase {
  friend class PropertiesSectionItem;

 public:
  //! Sets the value to a formatted string
  //! \todo review/remove (use core::format instead?)
  template <class T, class... EXTRA>
  void setValue(const T& value, EXTRA&&... extra) {
    setText(1, QString("%1").arg(value, std::forward<EXTRA>(extra)...));
  }

  //! Updates the value
  void setValue(const string& value) { setValue(QString::fromStdString(value)); }

 private:
  PropertyItem(PropertiesSectionItem* section, const string& name);
};

//! A property item which is bound to a core::Property
class BoundPropertyItem : public PropertyItemBase {
  friend class PropertiesSectionItem;

 public:
  //! The corresponding core::Property
  core::Property* property() const { return property_; }
  
  //! Returns `true` if it was set to a value different than the original one
  bool isModified() const override;

 private:
  BoundPropertyItem(PropertiesSectionItem* section, core::Property* property);

 private:
  core::Property* property_ = nullptr;
  string original_value_;
};

class PropertyItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
  
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

 private slots:
  void onComboBoxUpdate(int);
  
 private:
  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;
};

//! A visualizer/editor for a set of properties, grouped into collapsible sections
class PropertiesWidget : public QTreeWidget {
  Q_OBJECT

  friend PropertyItemDelegate;

 public:
  //! Creates a new, empty PropertiesWidget
  explicit PropertiesWidget(QWidget* parent);

  //! Creates a new properties section
  PropertiesSectionItem* addSection(const string& name);
  
  //! Creates a new properties section based on a core::PropertySet
  void addPropertiesSection(const string& name, core::PropertySet* property_set);
  
  //! Resets the columns widths to match the current content
  void autoSizeColumns();

  //! Returns `true` if any item was modified
  bool isModified() const;

 private:
  BoundPropertyItem* boundItemFromIndex(const QModelIndex& index) const;
};

}  // namespace core_ui
