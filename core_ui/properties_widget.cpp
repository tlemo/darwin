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

#include "properties_widget.h"

#include <QComboBox>
#include <QMessageBox>
#include <QMetaProperty>

namespace core_ui {

PropertiesWidget* PropertyItemBase::parentWidget() const {
  auto parent_widget = dynamic_cast<PropertiesWidget*>(treeWidget());
  CHECK(parent_widget != nullptr);
  return parent_widget;
}

PropertyItemBase* PropertyItemBase::parent() const {
  if(QTreeWidgetItem::parent() == nullptr) {
    return nullptr;
  }
  auto parent_item = dynamic_cast<PropertyItemBase*>(QTreeWidgetItem::parent());
  CHECK(parent_item != nullptr);
  return parent_item;
}

bool PropertyItemBase::isModified() const {
  for (int i = 0; i < childCount(); ++i) {
    if (dynamic_cast<PropertyItemBase*>(child(i))->isModified()) {
      return true;
    }
  }
  return false;
}

void PropertyItemBase::valueChanged() {
  // highlight the item
  auto new_font = font(0);
  new_font.setBold(isModified());
  setFont(0, new_font);
  setFont(1, new_font);  

  // propagate the notification to the parent, if any
  auto parent_item = parent();
  if (parent_item != nullptr) {
    parent_item->valueChanged();
  }
}

PropertiesWidget::PropertiesWidget(QWidget* parent) : QTreeWidget(parent) {
  setColumnCount(2);
  headerItem()->setText(0, "Property");
  headerItem()->setText(1, "Value");
  setHeaderHidden(false);

  setAlternatingRowColors(true);
  setUniformRowHeights(true);
  setColumnWidth(0, 150);
  setColumnWidth(1, 100);

  setEditTriggers(QAbstractItemView::AllEditTriggers);
  setItemDelegate(new PropertyItemDelegate(this));
}

PropertiesSectionItem* PropertiesWidget::addSection(const string& name) {
  auto section = new PropertiesSectionItem(name);
  addTopLevelItem(section);
  section->setFirstColumnSpanned(true);
  section->setExpanded(true);
  return section;
}

void PropertiesWidget::addPropertiesSection(const string& name,
                                            core::PropertySet* property_set) {
  auto section = addSection(name);
  for (core::Property* property : property_set->properties()) {
    section->addProperty(property);
  }
}

void PropertiesWidget::autoSizeColumns() {
  resizeColumnToContents(0);
  resizeColumnToContents(1);
}

bool PropertiesWidget::isModified() const {
  for (const auto& child : children()) {
    if (dynamic_cast<PropertyItemBase*>(child)->isModified()) {
      return true;
    }
  }
  return false;
}

BoundPropertyItem* PropertiesWidget::boundItemFromIndex(const QModelIndex& index) const {
  auto item = dynamic_cast<BoundPropertyItem*>(itemFromIndex(index));
  CHECK(item != nullptr);
  return item;
}

PropertyItem* PropertiesSectionItem::addProperty(const string& name) {
  return new PropertyItem(this, name);
}

BoundPropertyItem* PropertiesSectionItem::addProperty(core::Property* property) {
  return new BoundPropertyItem(this, property);
}

PropertiesSectionItem::PropertiesSectionItem(const string& name)
    : PropertyItemBase(nullptr) {
  setText(0, QString::fromStdString(name));
  setBackgroundColor(0, QColor(200, 220, 255));
}

PropertyItem::PropertyItem(PropertiesSectionItem* section, const string& name)
    : PropertyItemBase(section) {
  setText(0, QString::fromStdString(name));
}

bool BoundPropertyItem::isModified() const {
  return property_->value() != original_value_ || PropertyItemBase::isModified();
}

BoundPropertyItem::BoundPropertyItem(PropertiesSectionItem* section,
                                     core::Property* property)
    : PropertyItemBase(section), property_(property) {
  original_value_ = property_->value();
  setText(0, QString::fromStdString(property_->name()));
  setText(1, QString::fromStdString(original_value_));
  setToolTip(0, QString::fromStdString(property_->description()));
  setFlags(flags() | Qt::ItemIsEditable);
}

void PropertyItemDelegate::onComboBoxUpdate(int) {
  auto combo_box = dynamic_cast<QComboBox*>(sender());
  CHECK(combo_box != nullptr);
  emit commitData(combo_box);
}

// TODO: add a "restore default" button
QWidget* PropertyItemDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const {
  if (index.column() != 1)
    return nullptr;

  auto properties_widget = dynamic_cast<PropertiesWidget*>(this->parent());
  auto property = properties_widget->boundItemFromIndex(index)->property();
  auto known_values = property->knownValues();

  if (known_values.empty())
    return QStyledItemDelegate::createEditor(parent, option, index);

  auto editor = new QComboBox(parent);
  for (const auto& value : known_values)
    editor->addItem(QString::fromStdString(value));
  // force value update on combo box selection change
  connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxUpdate(int)));
  return editor;
}

void PropertyItemDelegate::setModelData(QWidget* editor,
                                        QAbstractItemModel* model,
                                        const QModelIndex& index) const {
  auto properties_widget = static_cast<PropertiesWidget*>(this->parent());
  auto bound_item = properties_widget->boundItemFromIndex(index);
  auto property = bound_item->property();

  // Qt voodoo, inspired from QStyledItemDelegate::setModelData()
  auto editor_property = editor->metaObject()->userProperty();
  string new_value = editor_property.read(editor).toString().toStdString();

  try {
    auto old_value = property->value();
    property->setValue(new_value);

    // use the final property round-trip value
    // (as a poor man's string form normalization)
    new_value = property->value();

    editor_property.write(editor, QString::fromStdString(new_value));
    QStyledItemDelegate::setModelData(editor, model, index);

    if (new_value != old_value)
      bound_item->valueChanged();
  } catch (const core::Exception&) {
    QMessageBox::warning(properties_widget,
                         "Invalid value",
                         QString::asprintf("'%s' is not a valid value for %s.",
                                           new_value.c_str(),
                                           property->name().c_str()));
  }
}

}  // namespace core_ui
