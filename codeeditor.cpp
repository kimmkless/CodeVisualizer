#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>

// C++ 语法高亮器实现
CppSyntaxHighlighter::CppSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 关键字格式
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = {
        "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bcontinue\\b",
        "\\bdefault\\b", "\\bdelete\\b", "\\bdo\\b", "\\belse\\b",
        "\\bfalse\\b", "\\bfor\\b", "\\bif\\b", "\\bin\\b",
        "\\bnew\\b", "\\bnullptr\\b", "\\breturn\\b", "\\bswitch\\b",
        "\\bthis\\b", "\\bthrow\\b", "\\btrue\\b", "\\btry\\b",
        "\\bwhile\\b", "\\bauto\\b", "\\bconst\\b", "\\bstatic\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 数据类型格式
    dataTypeFormat.setForeground(QColor(133, 153, 0));
    QStringList dataTypePatterns = {
        "\\bbool\\b", "\\bchar\\b", "\\bclass\\b", "\\bconst_cast\\b",
        "\\bdouble\\b", "\\bdynamic_cast\\b", "\\benum\\b", "\\bexplicit\\b",
        "\\bfriend\\b", "\\binline\\b", "\\bint\\b", "\\blong\\b",
        "\\bnamespace\\b", "\\boperator\\b", "\\bprivate\\b", "\\bprotected\\b",
        "\\bpublic\\b", "\\breinterpret_cast\\b", "\\bshort\\b", "\\bsigned\\b",
        "\\bsizeof\\b", "\\bstatic_cast\\b", "\\bstruct\\b", "\\btemplate\\b",
        "\\btypedef\\b", "\\btypename\\b", "\\bunion\\b", "\\bunsigned\\b",
        "\\bvirtual\\b", "\\bvoid\\b", "\\bvolatile\\b", "\\bfloat\\b", "\\bstd\\b"
    };
    for (const QString &pattern : dataTypePatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = dataTypeFormat;
        highlightingRules.append(rule);
    }

    // 函数格式
    functionFormat.setForeground(QColor(221, 74, 104));
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字格式
    numberFormat.setForeground(QColor(181, 137, 0));
    rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 预处理器格式
    preprocessorFormat.setForeground(QColor(100, 74, 155));
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 单行注释格式
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 多行注释格式
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // 字符串格式
    quotationFormat.setForeground(QColor(204, 102, 0));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
}

void CppSyntaxHighlighter::highlightBlock(const QString &text)
{
    // 应用每条规则
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 处理多行注释
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);
    
    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

// 代码编辑器实现
CodeEditor::CodeEditor(QWidget *parent) 
    : QPlainTextEdit(parent)
    , currentHighlightedLine(-1)
{
    lineNumberArea = new LineNumberArea(this);
    highlighter = new CppSyntaxHighlighter(document());
    
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    setLineWrapMode(QPlainTextEdit::NoWrap);
    
    QFont font;
    font.setFamily("Consolas");
    font.setFixedPitch(true);
    font.setPointSize(11);
    setFont(font);
    
    // 设置制表符宽度
    QFontMetrics metrics(font);
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));
}

void CodeEditor::highlightLine(int lineNumber)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!isReadOnly() && lineNumber >= 0) {
        QTextEdit::ExtraSelection selection;
        
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        for (int i = 0; i < lineNumber; ++i) {
            cursor.movePosition(QTextCursor::Down);
        }
        
        selection.cursor = cursor;
        extraSelections.append(selection);
        currentHighlightedLine = lineNumber;
    }
    
    setExtraSelections(extraSelections);
}

void CodeEditor::clearHighlight()
{
    setExtraSelections(QList<QTextEdit::ExtraSelection>());
    currentHighlightedLine = -1;
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(240, 240, 240));
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(120, 120, 120));
            if (blockNumber == currentHighlightedLine) {
                painter.fillRect(0, top, lineNumberArea->width(), fontMetrics().height(), QColor(Qt::yellow).lighter(160));
                painter.setPen(QColor(0, 0, 0));
            }
            painter.drawText(0, top, lineNumberArea->width() - 2, fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::highlightCurrentLine()
{
    if (isReadOnly() || currentHighlightedLine >= 0)
        return;
        
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    QTextEdit::ExtraSelection selection;
    
    QColor lineColor = QColor(Qt::yellow).lighter(190);
    
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    
    setExtraSelections(extraSelections);
}

void CodeEditor::setPlaceholderText(const QString &text)
{
    placeholderText = text;
    QPlainTextEdit::setPlaceholderText(text);
} 