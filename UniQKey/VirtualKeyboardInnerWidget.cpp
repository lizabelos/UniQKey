/*
* --------------------------------------------------------------
* Project: UniQKey
* Author: Liza Belos
* Year: 2023
* 
* Copyright (c) 2023. All rights reserved.
* This work is licensed under the terms of the MIT License.
* For a copy, see <https://opensource.org/licenses/MIT>.
* --------------------------------------------------------------
*
* NOTICE:
* This file is part of the original distribution of the UniQKey project. 
* All changes and redistributions of this file must retain this notice, 
* the list of contributors, and the entire copyright notice including the
* MIT License information.
* 
* DISCLAIMER:
* This software is provided 'as-is', without any express or implied warranty. 
* In no event will the authors be held liable for any damages arising from 
* the use of this software.
*/

#include "VirtualKeyboardInnerWidget.h"

#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QMainWindow>
#include <QDockWidget>

#include <unordered_set>

UniQKey::VirtualKeyboardInnerWidget::VirtualKeyboardInnerWidget() {
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    mMainLayout = new QStackedLayout();
    setLayout(mMainLayout);

    mKeyboardWithSuggestionsLayout = new QVBoxLayout();
    mKeyboardWithSuggestionsLayout->setSpacing(2);

    mSuggestionsLayout = new QHBoxLayout();
    mSuggestionsLayout->setSpacing(2);
    for (int i = 0; i < 3; i++) {
        mSuggestionButtons[i] = new QPushButton();
        mSuggestionsLayout->addWidget(mSuggestionButtons[i]);
        connect(mSuggestionButtons[i], &QPushButton::clicked, [=]() {
            if (mSuggestionButtons[i]->text() != "") {
                emit suggestionPressed(mSuggestionButtons[i]->text());
            }
        });
    }

    mKeyboardWithSuggestionsLayout->addLayout(mSuggestionsLayout);

    mKeyboardLayout = new QGridLayout();
    mKeyboardLayout->setSpacing(2);

    mKeyboardWithSuggestionsLayout->addLayout(mKeyboardLayout);

    mKeyboardWidget = new QWidget();
    mKeyboardWidget->setLayout(mKeyboardWithSuggestionsLayout);

    mMainLayout->addWidget(mKeyboardWidget);

    mConfigurationWidget = new VirtualKeyboardConfigurationWidget();
    mMainLayout->addWidget(mConfigurationWidget);
    connect(mConfigurationWidget, &VirtualKeyboardConfigurationWidget::requestKeyboard, [=](const QString &country, const QString &layout) {
        Keyboard keyboard = Keyboard::importKeyboard(country, layout);
        loadLayoutFromKeyboard(keyboard);
    });
    connect(mConfigurationWidget, &VirtualKeyboardConfigurationWidget::close, [=]() {
        mMainLayout->setCurrentWidget(mKeyboardWidget);
    });

    mOpenButton = new QPushButton();
    mOpenButton->setText("Open Keyboard");
    mMainLayout->addWidget(mOpenButton);
    connect(mOpenButton, &QPushButton::clicked, [=]() {
        mMainLayout->setCurrentWidget(mKeyboardWidget);
    });

    mMainLayout->setCurrentWidget(mKeyboardWidget);

    Keyboard keyboard = Keyboard::importKeyboard("US", "qwertyuiopasdfghjklzxcvbnm");
    loadLayoutFromKeyboard(keyboard);

}

bool UniQKey::VirtualKeyboardInnerWidget::loadLayoutFromKeyboard(const Keyboard& keyboard) {

    // empty the layout
    for (const auto& button : mButtons) {
        mKeyboardLayout->removeWidget(button);
        delete button;
    }
    mButtons.clear();

    for (const auto& key : keyboard.getKeys()) {
        addButtonFromKey(key);
    }

    return true;
}

void UniQKey::VirtualKeyboardInnerWidget::addButtonFromKey(const Key &key) {

    const int spanResolution = 4;

    int x = key.getX() * spanResolution;
    int y = key.getY() * spanResolution;
    int spanx = key.getXSpan() * spanResolution;
    int spany = key.getYSpan() * spanResolution;

    VirtualKeyboardButton *btn = new VirtualKeyboardButton(key);
    // fit the button to the size of the layout
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(btn, &VirtualKeyboardButton::virtualKeyPressed, this, &VirtualKeyboardInnerWidget::onVirtualKeyPressed);
    mKeyboardLayout->addWidget(btn, y, x, spany, spanx);
    mButtons.append(btn);
}

void UniQKey::VirtualKeyboardInnerWidget::onVirtualKeyPressed(VirtualKeyboardButton &button, const Key &key) {

    switch (key.getType()) {

        case KeyType::CONFIG:
            mMainLayout->setCurrentWidget(mConfigurationWidget);
            return;

        case KeyType::OPENCLOSE:
            triggerSetEnabled();
            return;

        case KeyType::SHIFT:
        case KeyType::ALT:
        case KeyType::CTRL:
            pressModifier(key);
            refreshModifiers(&button);
            break;

        default:
            break;
    }

    emit virtualKeyPressed(button, key); // admitting there is a direct connection

    switch (key.getType()) {

        case KeyType::SHIFT:
        case KeyType::ALT:
        case KeyType::CTRL:
            break;

        default:
            if (key.getCharacters().size() == 0) {
            } else {
                mKeyModifier = 0;
                refreshModifiers(&button);
            }
            break;
    }
}


void UniQKey::VirtualKeyboardInnerWidget::setEnabled(bool enabled) {
    mIsEnabled = enabled;
    if (enabled) {
        mMainLayout->setCurrentWidget(mKeyboardWidget);
    } else {
        mMainLayout->setCurrentWidget(mOpenButton);
    }
}

void UniQKey::VirtualKeyboardInnerWidget::triggerSetEnabled() {
    setEnabled(!mIsEnabled);
}

void UniQKey::VirtualKeyboardInnerWidget::refreshModifiers(QObject *toIgnore) {
    for (auto button : mButtons) {
        if (button->getKey().getType() == KeyType::REGULAR) {
            button->setCurrentKey(currentKeyType());
        } else if (button != toIgnore) {
            button->setChecked(isModifierPressed(button->getKey()));
        }
    }
}

void UniQKey::VirtualKeyboardInnerWidget::paintEvent(QPaintEvent *event) {
    qreal textHeight = 99999;
    for (auto button : mButtons) {
        textHeight = std::min(textHeight, button->recommendedTextSize());
    }
    for (auto button : mButtons) {
        button->setTextSize(textHeight);
    }
}