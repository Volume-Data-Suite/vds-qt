#pragma once

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class ExpandableSectionWidget : public QWidget {
    Q_OBJECT
private:
    QGridLayout mainLayout;
    QToolButton toggleButton;
    QFrame headerLine;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    int animationDuration{300};

public:
    explicit ExpandableSectionWidget(const QString& title = "", const int animationDuration = 0,
                     QWidget* parent = 0);
    void setContentLayout(QLayout& contentLayout);
};