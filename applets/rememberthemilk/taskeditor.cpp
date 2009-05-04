/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "taskeditor.h"

#include "taskitem.h"

#include <KComboBox>
#include <KLocale>
#include <KLineEdit>
#include <KPushButton>
#include <KDebug>

#include <QPainter>
#include <QStringList>
#include <QLabel>
#include <QDateTime>

#include <Plasma/Theme>
#include <Plasma/Animator>
#include <Plasma/IconWidget>
#include <Plasma/Service>

TaskEditor::TaskEditor(Plasma::DataEngine* engine, QGraphicsWidget* parent)
  : QGraphicsWidget(parent),
  m_engine(engine),
  m_service(0)
{
  mainLayout = new QGraphicsGridLayout(this);
  nameEdit = new Plasma::LineEdit(this);
  dateEdit = new Plasma::LineEdit(this);
  tagsEdit = new Plasma::LineEdit(this);
  priorityEdit = new Plasma::ComboBox(this);
  completeBox = new Plasma::CheckBox(this);

  nameLabel = new Plasma::Label(this);
  nameLabel->setText(i18n("Name:"));
  dateLabel = new Plasma::Label(this);
  dateLabel->setText(i18n("Due:"));
  tagsLabel = new Plasma::Label(this);
  tagsLabel->setText(i18n("Tags:"));
  priorityLabel = new Plasma::Label(this);
  priorityLabel->setText(i18n("Priority:"));
  completeLabel = new Plasma::Label(this);
  completeLabel->setText(i18n("Complete:"));

  QStringList priorityStrings;
  priorityStrings << i18n("Top Priority") << i18n("Medium Priority") << i18n("Low Priority") << i18n("No Priority");
  priorityEdit->nativeWidget()->addItems(priorityStrings);

  saveChangesButton = new Plasma::PushButton(this);
  connect(saveChangesButton, SIGNAL(clicked()), this, SIGNAL(requestSaveChanges()));
  connect(saveChangesButton, SIGNAL(clicked()), this, SLOT(saveChanges()));
  discardChangesButton = new Plasma::PushButton(this);
  connect(discardChangesButton, SIGNAL(clicked()), this, SIGNAL(requestDiscardChanges()));
  connect(discardChangesButton, SIGNAL(clicked()), this, SLOT(discardChanges()));


  saveChangesButton->setText(i18n("Update Task"));
  saveChangesButton->nativeWidget()->setIcon(KIcon("dialog-ok-apply"));

  discardChangesButton->setText(i18n("Discard Changes"));
  discardChangesButton->nativeWidget()->setIcon(KIcon("dialog-cancel"));

  mainLayout->addItem(nameLabel, 0, 0);
  mainLayout->addItem(nameEdit, 0, 1);
  
  mainLayout->addItem(dateLabel, 1, 0);
  mainLayout->addItem(dateEdit, 1, 1);
  
  mainLayout->addItem(tagsLabel, 2, 0);
  mainLayout->addItem(tagsEdit, 2, 1);

  mainLayout->addItem(priorityLabel, 3, 0);
  mainLayout->addItem(priorityEdit, 3, 1);


  mainLayout->addItem(completeLabel, 4, 0);
  mainLayout->addItem(completeBox, 4, 1);
  
  //mainLayout->setColumnStretchFactor(2, 1);
  mainLayout->setRowStretchFactor(6, 1);

  mainLayout->addItem(saveChangesButton, 7, 0, 1, 2);
  mainLayout->addItem(discardChangesButton, 8, 0, 1, 2);

  opacity = .9;

  setLayout(mainLayout);
}

TaskEditor::~TaskEditor()
{
  // Service is parented to the engine and is thus deleted automatically
}


void TaskEditor::keyPressEvent(QKeyEvent* event) {
  kDebug() << event->key();
  if (event->key() == Qt::Key_Escape) {
    //emit requestDiscardChanges(); // Only works when in line edits... i.e. not good
    //discardChanges();
  }
  QGraphicsItem::keyPressEvent(event);
}


void TaskEditor::setModelIndex(QModelIndex index) {
  m_index = index.data(Qt::RTMTaskIdRole).toULongLong();
  nameEdit->nativeWidget()->clear();
  nameEdit->nativeWidget()->setClickMessage(index.data(Qt::RTMNameRole).toString());
  dateEdit->nativeWidget()->clear();
  dateEdit->nativeWidget()->setClickMessage(index.data(Qt::RTMDueRole).toDate().toString(Qt::DefaultLocaleShortDate)); //FIXME: Allow times within a date
  tagsEdit->nativeWidget()->clear();
  tagsEdit->nativeWidget()->setClickMessage(index.data(Qt::RTMTagsRole).toStringList().join(", "));
  priorityEdit->nativeWidget()->setCurrentIndex((index.data(Qt::RTMPriorityRole).toInt()-1) % 4);
  m_priority = priorityEdit->nativeWidget()->currentIndex();
  completeBox->setChecked(index.data(Qt::RTMCompletedRole).toBool());
  
  if (m_service)
    m_service->deleteLater();
  m_service = m_engine->serviceForSource("Task:" + QString::number(m_index));
  connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), SIGNAL(jobFinished(Plasma::ServiceJob*)));
}

void TaskEditor::discardChanges() {
  startAnimation(fullSize, false);
}

void TaskEditor::saveChanges() {
  if (!m_service)
    return; // No index (and hence no task) has been set, or something is really wrong.
  
  if (!nameEdit->text().isEmpty()) {
    kDebug() << "Name Change: " << nameEdit->text();
    KConfigGroup cg = m_service->operationDescription("setName");
    cg.writeEntry("name", nameEdit->text());
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (!dateEdit->text().isEmpty()) {
    kDebug() << "Date Change: " << dateEdit->text();
    KConfigGroup cg = m_service->operationDescription("setDueText");
    cg.writeEntry("dueText", dateEdit->text());
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (!tagsEdit->text().isEmpty()) {
    QStringList tags = tagsEdit->text().split(",");
    KConfigGroup cg = m_service->operationDescription("setTags");
    cg.writeEntry("tags", tags);
    emit jobStarted(m_service->startOperationCall(cg));
  }

  if (priorityEdit->nativeWidget()->currentIndex() != m_priority) {
    KConfigGroup cg = m_service->operationDescription("setPriority");
    cg.writeEntry("priority", priorityEdit->nativeWidget()->currentIndex() + 1);
    emit jobStarted(m_service->startOperationCall(cg));
  }
  
  if (completeBox->isChecked()) {
    KConfigGroup cg = m_service->operationDescription("setCompleted");
    cg.writeEntry("completed", true);
    emit jobStarted(m_service->startOperationCall(cg));
  }

  startAnimation(fullSize, false);
}


void TaskEditor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  Q_UNUSED(widget)
  QColor wash = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
  wash.setAlphaF(opacity);
  painter->setBrush(wash);
  painter->drawRect(option->exposedRect);
}


void TaskEditor::setFullSize(QSizeF size) {
  fullSize = size;
  resize(fullSize);
}


void TaskEditor::startAnimation(QSizeF endSize, bool show) {
    appearing = show;
    if (appearing) {
      opacity = 0;
      foreach(QGraphicsItem* child, childItems())
	child->show();
    }
    else
      this->show();
    this->show();
    fullSize = endSize;
    resize(fullSize);
    if (show)
      Plasma::Animator::self()->customAnimation(10, 100, Plasma::Animator::EaseInCurve, this, "onAnimValueChanged");
    else
      Plasma::Animator::self()->customAnimation(10, 100, Plasma::Animator::EaseOutCurve, this, "onAnimValueChanged");
}

void TaskEditor::onAnimValueChanged(qreal value) {
    if (value == 1.0) {
        animationFinished();
        return;
    }


  if (appearing)
    opacity = value*.9;
  else
    opacity = (1-value)*.9;

    update();
}

void TaskEditor::animationFinished() {
    if (appearing) {
      setPos(0, 0);
      resize(fullSize);
      opacity = .9;
    }
    else {
      opacity = 0;
      hide();
    }

    update();
}

#include "taskeditor.moc"
