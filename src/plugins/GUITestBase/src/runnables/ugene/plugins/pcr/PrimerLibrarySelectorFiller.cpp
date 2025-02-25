/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <drivers/GTMouseDriver.h>
#include <primitives/GTWidget.h>

#include <QApplication>

#include "GTUtilsPrimerLibrary.h"
#include "PrimerLibrarySelectorFiller.h"

namespace U2 {
using namespace HI;

PrimerLibrarySelectorFiller::PrimerLibrarySelectorFiller(HI::GUITestOpStatus &os, int number, bool doubleClick)
    : Filler(os, "PrimerLibrarySelector"), number(number), doubleClick(doubleClick) {
}

#define GT_CLASS_NAME "GTUtilsDialog::PrimerLibrarySelectorFiller"
#define GT_METHOD_NAME "run"
void PrimerLibrarySelectorFiller::commonScenario() {
    QWidget *dialog = GTWidget::getActiveModalWidget(os);
    QAbstractButton *okButton = GTUtilsDialog::buttonBox(os, dialog)->button(QDialogButtonBox::Ok);
    CHECK_SET_ERR(!okButton->isEnabled(), "the OK button is enabled, but is expected to be disabled");

    int librarySize = GTUtilsPrimerLibrary::librarySize(os);
    int primerNumber = number == -1 ? librarySize - 1 : number;
    GTMouseDriver::moveTo(GTUtilsPrimerLibrary::getPrimerPoint(os, primerNumber));
    GTMouseDriver::click();
    CHECK_SET_ERR(okButton->isEnabled(), "the OK button is disabled, but is expected to be enabled");

    if (doubleClick) {
        GTMouseDriver::doubleClick();
    } else {
        GTWidget::click(os, okButton);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}  // namespace U2
