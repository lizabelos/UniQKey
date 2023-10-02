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

#ifndef VIRTUALKEYBOARDBUTTON_H
#define VIRTUALKEYBOARDBUTTON_H

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPointer>
#include <QLabel>
#include <QFont>
#include <QColor>
#include <QList>
#include <QComboBox>

#include <unordered_set>
#include <utility>

#include "Keyboard.h"

namespace UnivKbd {

    class VirtualKeyboardSpecialsWidget : public QWidget {
    Q_OBJECT

    friend class VirtualKeyboardButton;

    private:
        VirtualKeyboardSpecialsWidget(QStringList specials, QWidget *parent);

        void paintEvent(QPaintEvent *event) override;

    signals:
        void specialKeyPressed(const QString &key);

    private:
        QStringList mSpecials;
        QPointer<QHBoxLayout> mLayout;
        QList<QPointer<QPushButton>> mButtons;
        QWidget *mDirectParent;
    };

    class VirtualKeyboardButton : public QAbstractButton {
    Q_OBJECT

    public:
        VirtualKeyboardButton(const Key &key, std::shared_ptr<QFont> font, QWidget *parent);
        ~VirtualKeyboardButton() override;

        void setCurrentKey(int index);

        inline int getCurrentKey() const {
            return mCurrentKey;
        }

        inline const Key &getKey() const {
            return mKey;
        }

        //qreal recommendedTextSize() const;

        //inline void setTextSize(float size) {
        //    mTextSize = size;
        //}

        inline void setFont(std::shared_ptr<QFont> font) {
            mFont = std::move(font);
        }


    protected:
        void paintEvent(QPaintEvent *event) override;

    signals:
        void virtualKeyPressed(VirtualKeyboardButton &button, const Key &key);

        void specialKeyPressed(VirtualKeyboardButton &button, const Key &key, const QString &special);

    private slots:
        void virtualButtonPressed();

    private:
        Key mKey;
        QString mKeyString[3];

        int mCurrentKey;

        QPixmap mPixmap;

        QList<QPointer<VirtualKeyboardSpecialsWidget>> mSpecialsWidget;
        std::shared_ptr<QFont> mFont;

    };

}

#endif