/*
* ============================================================================
*
* Copyright [2010] Sigmoid Solutions limited
*
* Description:  Main window for dedup application
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

#ifndef MAIDSAFE_DEDUP_ANALYSER_WIDGETS_ANALYSER_H_
#define MAIDSAFE_DEDUP_ANALYSER_WIDGETS_ANALYSER_H_

#include <boost/shared_ptr.hpp>
#include <QWidget>

namespace Ui { class Analyser; }

namespace maidsafe {

//  class SpeedoMeter;

class AnalyserWidget : public QWidget {
    Q_OBJECT
 public:
  explicit AnalyserWidget(QWidget *parent);
  ~AnalyserWidget() {}
 signals:
  void StopScanning();
 public slots:  // NOLINT (Fraser)
  void StopButtonClicked();
 private:
  boost::shared_ptr<Ui::Analyser> ui_analyser_;
//  SpeedoMeter *spacePercentageMeter;
//  SpeedoMeter *dupePercentageMeter;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_DEDUP_ANALYSER_WIDGETS_ANALYSER_H_
