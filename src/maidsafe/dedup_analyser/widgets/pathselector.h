/*
* ============================================================================
*
* Copyright [2010] Sigmoid Solutions limited
*
* Description:  Locate and allow selection of drives and paths
* Version:      1.0
* Created:      2010, 21 / 12
* Revision:     none
* Author:       Saidle
* Company:      Sigmoid Solutions
*
* The following source code is property of Sigmoid Solutions and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.sigmoidsolutions.com
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of Sigmoid
* Solutions.
* ============================================================================
*/
#ifndef PATHSELECTOR_H
#define PATHSELECTOR_H

#include <QWidget>

namespace Ui {
    class PathSelector;
}

class QFileSystemModel;

class PathSelector : public QWidget
{
    Q_OBJECT

public:
    explicit PathSelector(QWidget *parent = 0);
    ~PathSelector();

signals:
    void analyseNow();    

private:
    /* 
    * allocates memory for members
    */
    void createViewItems();

    /*
    * Filters dupes before adding to list
    */
    void addItemToList(QString);

    private slots:
        void itemSelected();
        void itemDeselected();

private:
    Ui::PathSelector *ui;
    QFileSystemModel *fileModel_;
};

#endif // PATHSELECTOR_H
