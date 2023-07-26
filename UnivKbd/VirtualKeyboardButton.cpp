/*
* --------------------------------------------------------------
* Project: UnivKbd
* Author: Liza Belos
* Year: 2023
* 
* Copyright (c) 2023. All rights reserved.
* This work is licensed under the terms of the MIT License.
* For a copy, see <https://opensource.org/licenses/MIT>.
* --------------------------------------------------------------
*
* NOTICE:
* This file is part of the original distribution of the UnivKbd project.
* All changes and redistributions of this file must retain this notice, 
* the list of contributors, and the entire copyright notice including the
* MIT License information.
* 
* DISCLAIMER:
* This software is provided 'as-is', without any express or implied warranty. 
* In no event will the authors be held liable for any damages arising from 
* the use of this software.
*/

#include "VirtualKeyboardButton.h"

#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QMainWindow>
#include <QDockWidget>
#include <QPainter>

#include <unordered_set>

UnivKbd::VirtualKeyboardButton::VirtualKeyboardButton(const Key &key, QWidget *parent) : QAbstractButton(parent), mKey(key) {

    switch (key.getType()) {

        case KeyType::REGULAR:
            mKeyString[0] = key.toString(0);
            mKeyString[1] = key.toString(1);
            mKeyString[2] = key.toString(2);

            break;

        default:
            mKeyString[0] = key.toString();
            mKeyString[1] = key.toString();
            mKeyString[2] = key.toString();
            setCheckable(true);
            break;

    }

    connect(this, &QAbstractButton::pressed, this, &VirtualKeyboardButton::virtualButtonPressed);

    setCurrentKey(0);

    // set minimum height
    setMinimumHeight(50);


    QString svgPath = ":/" + QString::fromStdString(std::to_string((int)mKey.getType())) + ".svg";
    QFile svgFile(svgPath);

    if (svgFile.open(QIODevice::ReadOnly)) {
        mPixmap = QPixmap(svgPath);
    }

    for (int i = 0; i < key.getCharacters().size(); i++) {
        if (key.getSpecials(i).size() == 0) {
            mSpecialsWidget.append(nullptr);
        } else {
            QPointer<VirtualKeyboardSpecialsWidget> specialsWidget = new VirtualKeyboardSpecialsWidget(key.getSpecials(i), this);
            specialsWidget->hide();
            mSpecialsWidget.append(specialsWidget);
            connect(specialsWidget, &VirtualKeyboardSpecialsWidget::specialKeyPressed, [=](const QString &special) {
                emit specialKeyPressed(*this, mKey, special);
            });
        }
    }

}

UnivKbd::VirtualKeyboardButton::~VirtualKeyboardButton() {

}

void UnivKbd::VirtualKeyboardButton::virtualButtonPressed() {
    emit virtualKeyPressed(*this, mKey);
}

void UnivKbd::VirtualKeyboardButton::setCurrentKey(int index) {
    int maxIndex = 0;
    for (; maxIndex < 2; maxIndex++) {
        if (mKeyString[maxIndex + 1] == "") {
            break;
        }
    }

    mCurrentKey = std::min(maxIndex, index);
    setText(mKeyString[index]);
}

void UnivKbd::VirtualKeyboardButton::paintEvent(QPaintEvent *event) {

    // unused parameter
    (void)event;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the background according to the button's state (hover, pressed, checked, etc.)
    if (isDown())
    {
        painter.fillRect(rect(), QColor(0xCC, 0xCC, 0xCC));
    }
    else if (isChecked())
    {
        painter.fillRect(rect(), QColor(0xFF, 0xCC, 0x99));
    }
    else if (underMouse())
    {
        painter.fillRect(rect(), QColor(0xCC, 0xCC, 0xCC));
    }
    else
    {
        painter.fillRect(rect(), QColor(0xFF, 0xFF, 0xFF));
    }

    if (underMouse()) {
        if (mSpecialsWidget.size() > mCurrentKey && mSpecialsWidget[mCurrentKey] != nullptr) {
            mSpecialsWidget[mCurrentKey]->show();
        }
    } else {
        if (mSpecialsWidget.size() > mCurrentKey && mSpecialsWidget[mCurrentKey] != nullptr) {
            if (!mSpecialsWidget[mCurrentKey]->underMouse()) {
                mSpecialsWidget[mCurrentKey]->hide();
            }
        }
    }

    if (mTextSize == 0) {
        mTextSize = recommendedTextSize();
    }

    if (mPixmap.isNull()) {
        QFont font = painter.font();
        font.setPointSizeF(mTextSize);
        painter.setFont(font);

        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, text());
    } else {
        QRect svgSize = mPixmap.rect();
        qreal svgHeight = mTextSize;
        qreal svgWidth = svgHeight * svgSize.width() / svgSize.height();

        QRect svgRect(rect().center().x() - svgWidth / 2, rect().center().y() - svgHeight / 2, svgWidth, svgHeight);
        painter.drawPixmap(svgRect, mPixmap);
    }

}

qreal UnivKbd::VirtualKeyboardButton::recommendedTextSize() const {
    if (mPixmap.isNull()) {
        // Calculate the font size based on the button's dimensions and text width
        QFont font;
        qreal maxFontSize = qMin(0.6 * rect().width(), 0.6 * rect().height()); // Maximum font size based on button width and height
        qreal fontPointSize = 0.8 * maxFontSize; // Initial font size based on the maximum font size
        font.setPointSizeF(fontPointSize);

        QFontMetrics fontMetrics(font);
        qreal textWidth = fontMetrics.horizontalAdvance(text()); // Width of the button's text

        // Adjust the font size to fit the text within the button's width
        if (textWidth > 0.6 * rect().width())
        {
            fontPointSize *= (0.6 * rect().width()) / textWidth;
        }

        // Adjust the font size to fit the text within the button's height
        if (fontPointSize > maxFontSize)
        {
            fontPointSize = maxFontSize;
        }

        return fontPointSize;

    } else {
        QRect svgSize = mPixmap.rect();
        // make it's height 0.6 of the button's height
        qreal svgHeight = 0.6 * rect().height();
        qreal svgWidth = svgHeight * svgSize.width() / svgSize.height();

        if (svgWidth > 0.6 * rect().width()) {
            svgWidth = 0.6 * rect().width();
            svgHeight = svgWidth * svgSize.height() / svgSize.width();
        }

        return svgHeight;
    }
}

UnivKbd::VirtualKeyboardSpecialsWidget::VirtualKeyboardSpecialsWidget(QStringList specials, QWidget *parent) : QWidget(parent->parentWidget()), mDirectParent(parent), mSpecials(specials) {
    //QPointer<QHBoxLayout> mLayout;
    //QList<QPointer<QPushButton>> mButtons;
    qDebug() << "Creating specials widget with parent " << parent;

    mLayout = new QHBoxLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    for (int i = 0; i < specials.size(); i++) {
        QPointer<QPushButton> button = new QPushButton(specials[i]);
        mButtons.push_back(button);
        mLayout->addWidget(button);
        connect(button, &QPushButton::pressed, [=]() {
            emit specialKeyPressed(specials[i]);
        });
    }

    setLayout(mLayout);

    setStyleSheet("background-color: red;");
}

void UnivKbd::VirtualKeyboardSpecialsWidget::paintEvent(QPaintEvent *event) {
    // position the widget in the top center of the parent rect on its parent widget
    QRect parentDrawRect = mDirectParent->rect().translated(mDirectParent->pos());
    QRect drawRect = rect().translated(parentDrawRect.center().x() - rect().center().x(), parentDrawRect.top() - rect().height());
    move(drawRect.topLeft());

    qDebug() << "Painting specials widget with parent " << mDirectParent << " at " << drawRect.topLeft();

    // update parent
    mDirectParent->update();
}

