#include "SimpleTextEditor.h"
#include "VirtualKeyboard.h"

UniQKey::SimpleTextEditor::SimpleTextEditor(QWidget *parent) {

    /*** Text Document ***/
    mTextDocument = new QTextDocument(this);
    // Set the page size to A4
    mTextDocument->setPageSize(QSizeF(QSize(595, 842)));
    // Margin
    mTextDocument->setDocumentMargin(50);

    // Set the default font
    QFont font;
    font.setFamily("Times New Roman");
    font.setPointSize(12);
    mTextDocument->setDefaultFont(font);

    /*** Text Editor ***/
    mTextEditor = new QTextEdit(this);
    mTextEditor->setDocument(mTextDocument);
    // Set the page size to A4
    mTextEditor->setFixedSize(QSize(595, 842));
    connect(mTextEditor, &QTextEdit::currentCharFormatChanged, this, &SimpleTextEditor::updateFormatActions);


    VirtualKeyboard *keyboard = new VirtualKeyboard(mTextEditor);

    /*** Font Actions ***/
    boldAction = new QAction("Bold", this);
    italicAction = new QAction("Italic", this);
    underlineAction = new QAction("Underline", this);

    boldAction->setCheckable(true);
    italicAction->setCheckable(true);
    underlineAction->setCheckable(true);

    connect(boldAction, &QAction::triggered, this, &SimpleTextEditor::makeTextBold);
    connect(italicAction, &QAction::triggered, this, &SimpleTextEditor::makeTextItalic);
    connect(underlineAction, &QAction::triggered, this, &SimpleTextEditor::makeTextUnderlined);

    // Create a "Format" menu and add the actions
    QMenu *formatMenu = menuBar()->addMenu("Format");
    formatMenu->addAction(boldAction);
    formatMenu->addAction(italicAction);
    formatMenu->addAction(underlineAction);

    // Create a toolbar and add the actions
    QToolBar *toolbar = addToolBar("Format");
    toolbar->addAction(boldAction);
    toolbar->addAction(italicAction);
    toolbar->addAction(underlineAction);

    // Zoom slider in status bar
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(1, 5);
    zoomSlider->setValue(3);
    zoomSlider->setToolTip("Zoom");
    connect(zoomSlider, &QSlider::valueChanged, this, &SimpleTextEditor::zoom);

    statusBar()->addWidget(zoomSlider);

    /*** Scroll Area ***/
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(mTextEditor);
    // center the text editor
    scrollArea->setAlignment(Qt::AlignCenter);

    /*** This Window **/
    setMinimumSize(QSize(795, 1042));
    setCentralWidget(scrollArea);
}

void UniQKey::SimpleTextEditor::makeTextBold() {
    QFont font = mTextEditor->currentFont();
    font.setBold(!font.bold());
    mTextEditor->setCurrentFont(font);
}

void UniQKey::SimpleTextEditor::makeTextItalic() {
    QFont font = mTextEditor->currentFont();
    font.setItalic(!font.italic());
    mTextEditor->setCurrentFont(font);
}

void UniQKey::SimpleTextEditor::makeTextUnderlined() {
    QFont font = mTextEditor->currentFont();
    font.setUnderline(!font.underline());
    mTextEditor->setCurrentFont(font);
}

void UniQKey::SimpleTextEditor::updateFormatActions(const QTextCharFormat &format) {
    boldAction->setChecked(format.font().bold());
    italicAction->setChecked(format.font().italic());
    underlineAction->setChecked(format.font().underline());
}

void UniQKey::SimpleTextEditor::wheelEvent(QWheelEvent *event) {
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}

void UniQKey::SimpleTextEditor::zoom(int level) {
    qreal factor = level / 3.0;
    mZoomFactor = factor;
    QFont font = mTextEditor->font();
    font.setPointSizeF(12 * factor);
    mTextEditor->setFont(font);
    mTextEditor->setFixedSize(QSize(595 * mZoomFactor, 842 * mZoomFactor));


}

void UniQKey::SimpleTextEditor::zoomIn() {
    mZoomFactor += 0.1;
    QFont font = mTextEditor->font();
    font.setPointSizeF(12 * mZoomFactor);
    mTextEditor->setFont(font);
    mTextEditor->setFixedSize(QSize(595 * mZoomFactor, 842 * mZoomFactor));

}

void UniQKey::SimpleTextEditor::zoomOut() {
    mZoomFactor -= 0.1;
    QFont font = mTextEditor->font();
    font.setPointSizeF(12 * mZoomFactor);
    mTextEditor->setFont(font);
    mTextEditor->setFixedSize(QSize(595 * mZoomFactor, 842 * mZoomFactor));

}