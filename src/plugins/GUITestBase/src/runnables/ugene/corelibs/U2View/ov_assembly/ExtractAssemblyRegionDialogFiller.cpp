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

#include "ExtractAssemblyRegionDialogFiller.h"
#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTWidget.h>

#include <QApplication>

namespace U2 {
using namespace HI;

ExtractAssemblyRegionDialogFiller::ExtractAssemblyRegionDialogFiller(HI::GUITestOpStatus &os, const QString &filepath, const U2Region &region, const QString &format)
    : Filler(os, "ExtractAssemblyRegionDialog"), filepath(filepath), regionToExtract(region), format(format) {
}

#define GT_CLASS_NAME "ExtractAssemblyRegionDialogFiller"

#define GT_METHOD_NAME "commonScenario"
void ExtractAssemblyRegionDialogFiller::commonScenario() {
    QWidget *widget = GTWidget::getActiveModalWidget(os);

    QComboBox *docFormatCB = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "documentFormatComboBox", widget));
    CHECK_SET_ERR(docFormatCB != nullptr, "docFormatCB widget is NULL");
    GTComboBox::selectItemByText(os, docFormatCB, format);

    QLineEdit *startLineEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "start_edit_line", widget));
    CHECK_SET_ERR(startLineEdit != nullptr, "startLineEdit widget is NULL");
    GTLineEdit::setText(os, startLineEdit, QString::number(regionToExtract.startPos));

    QLineEdit *endLineEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "end_edit_line", widget));
    CHECK_SET_ERR(endLineEdit != nullptr, "endLineEdit widget is NULL");
    GTLineEdit::setText(os, endLineEdit, QString::number(regionToExtract.endPos()));

    QLineEdit *filepathLineEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "filepathLineEdit", widget));
    CHECK_SET_ERR(filepathLineEdit != nullptr, "filepathLineEdit widget is NULL");
    GTLineEdit::setText(os, filepathLineEdit, filepath);

    GTUtilsDialog::clickButtonBox(os, widget, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}  // namespace U2
