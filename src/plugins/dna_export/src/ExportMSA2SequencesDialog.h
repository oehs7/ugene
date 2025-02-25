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

#ifndef _U2_EXPORT_MSA2SEQ_DIALOG_H_
#define _U2_EXPORT_MSA2SEQ_DIALOG_H_

#include <ui_ExportMSA2SequencesDialog.h>

#include <QDialog>

#include <U2Core/global.h>

namespace U2 {

class SaveDocumentController;

class ExportMSA2SequencesDialog : public QDialog, private Ui_ExportMSA2SequencesDialog {
    Q_OBJECT
public:
    ExportMSA2SequencesDialog(const QString &defaultDir, const QString &defaultFilename, QWidget *p);

    virtual void accept();

public:
    QString url;
    QString defaultDir;
    QString defaultFileName;
    DocumentFormatId format;
    bool trimGapsFlag;
    bool addToProjectFlag;

private:
    void initSaveController();

    SaveDocumentController *saveController;
};

}  // namespace U2

#endif
