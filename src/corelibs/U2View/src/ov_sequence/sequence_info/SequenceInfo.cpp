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

#include "SequenceInfo.h"

#include <QLabel>
#include <QVBoxLayout>

#include <U2Core/AnnotationSelection.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVSequenceWidget.h>
#include <U2View/AnnotatedDNAView.h>

#include "CodonOccurTask.h"

namespace U2 {

const int SequenceInfo::COMMON_STATISTICS_TABLE_CELLSPACING = 5;
const QString SequenceInfo::CAPTION_SEQ_REGION_LENGTH = "Length: ";

const QString SequenceInfo::CAPTION_SUFFIX_DS_DNA = "dsDNA:";
const QString SequenceInfo::CAPTION_SUFFIX_SS_DNA = "ssDNA:";
const QString SequenceInfo::CAPTION_SUFFIX_DS_RNA = "dsRNA:";
const QString SequenceInfo::CAPTION_SUFFIX_SS_RNA = "ssRNA:";

// nucl
const QString SequenceInfo::CAPTION_SEQ_GC_CONTENT = "GC content: ";
const QString SequenceInfo::CAPTION_SEQ_NUCL_MOLECULAR_WEIGHT = "Molecular weight: ";
const QString SequenceInfo::CAPTION_SEQ_EXTINCTION_COEFFICIENT = "Extinction coefficient: ";
const QString SequenceInfo::CAPTION_SEQ_MELTING_TEMPERATURE = "Melting temperature: ";

const QString SequenceInfo::CAPTION_SEQ_NMOLE_OD = "nmole/OD<sub>260</sub>: ";
const QString SequenceInfo::CAPTION_SEQ_MG_OD = QChar(0x3BC) + QString("g/OD<sub>260</sub>: ");  // 0x3BC - greek 'mu'

// amino
const QString SequenceInfo::CAPTION_SEQ_AMINO_MOLECULAR_WEIGHT = "Molecular weight: ";
const QString SequenceInfo::CAPTION_SEQ_ISOELECTIC_POINT = "Isoelectic point: ";

const QString SequenceInfo::CHAR_OCCUR_GROUP_ID = "char_occur_group";
const QString SequenceInfo::DINUCL_OCCUR_GROUP_ID = "dinucl_occur_group";
const QString SequenceInfo::CODON_OCCUR_GROUP_ID = "codon_occur_group";
const QString SequenceInfo::AMINO_ACID_OCCUR_GROUP_ID = "amino_acid_occur_group";
const QString SequenceInfo::STAT_GROUP_ID = "stat_group";

SequenceInfo::SequenceInfo(AnnotatedDNAView *_annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView), savableWidget(this, GObjectViewUtils::findViewByName(_annotatedDnaView->getName())) {
    SAFE_POINT(0 != annotatedDnaView, "AnnotatedDNAView is NULL!", );

    updateCurrentRegions();
    initLayout();
    connectSlots();
    updateData();

    U2WidgetStateStorage::restoreWidgetState(savableWidget);
}

void SequenceInfo::initLayout() {
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);

    // Common statistics
    QWidget *statisticLabelContainer = new QWidget(this);
    statisticLabelContainer->setLayout(new QHBoxLayout);
    statisticLabelContainer->layout()->setContentsMargins(0, 0, 0, 0);

    statisticLabel = new QLabel(statisticLabelContainer);
    statisticLabel->installEventFilter(this);
    statisticLabel->setMinimumWidth(1);
    statisticLabel->setObjectName("Common Statistics");
    statisticLabelContainer->layout()->addWidget(statisticLabel);

    statsWidget = new ShowHideSubgroupWidget(STAT_GROUP_ID, tr("Common Statistics"), statisticLabelContainer, true);

    mainLayout->addWidget(statsWidget);

    // Characters occurrence
    charOccurLabel = new QLabel(this);
    charOccurLabel->setObjectName("characters_occurrence_label");
    charOccurWidget = new ShowHideSubgroupWidget(
        CHAR_OCCUR_GROUP_ID, tr("Characters Occurrence"), charOccurLabel, true);
    charOccurWidget->setObjectName("Characters Occurrence");
    mainLayout->addWidget(charOccurWidget);

    // Dinucleotides
    dinuclLabel = new QLabel(this);
    dinuclWidget = new ShowHideSubgroupWidget(
        DINUCL_OCCUR_GROUP_ID, tr("Dinucleotides"), dinuclLabel, false);
    dinuclWidget->setObjectName("Dinucleotides");
    mainLayout->addWidget(dinuclWidget);

    // Codons.
    codonLabel = new QLabel(this);
    codonWidget = new ShowHideSubgroupWidget(CODON_OCCUR_GROUP_ID, tr("Codons"), codonLabel, false);
    codonWidget->setObjectName("options_panel_codons_widget");
    mainLayout->addWidget(codonWidget);

    // Amino acids.
    aminoAcidLabel = new QLabel(this);
    aminoAcidWidget = new ShowHideSubgroupWidget(AMINO_ACID_OCCUR_GROUP_ID, tr("Amino acids"), aminoAcidLabel, false);
    aminoAcidWidget->setObjectName("options_panel_amino_acids_widget");
    mainLayout->addWidget(aminoAcidWidget);

    // Make some labels selectable by a user (so he could copy them)
    charOccurLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dinuclLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    codonLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    aminoAcidLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    statisticLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    updateLayout();
}

void SequenceInfo::updateLayout() {
    updateCharOccurLayout();
    updateDinuclLayout();
    updateCodonOccurLayout();
}

namespace {

/** Formats long number by separating each three digits */
QString getFormattedLongNumber(qint64 num) {
    QString result;

    int DIVIDER = 1000;
    do {
        int lastThreeDigits = num % DIVIDER;

        QString digitsStr = QString::number(lastThreeDigits);

        // Fill with zeros if the digits are in the middle of the number
        if (num / DIVIDER != 0) {
            digitsStr = QString("%1").arg(digitsStr, 3, '0');
        }

        result = digitsStr + " " + result;

        num /= DIVIDER;
    } while (num != 0);

    return result;
}

}  // namespace

void SequenceInfo::updateCharOccurLayout() {
    ADVSequenceObjectContext *activeSequenceContext = annotatedDnaView->getActiveSequenceContext();
    if (0 != activeSequenceContext) {
        const DNAAlphabet *activeSequenceAlphabet = activeSequenceContext->getAlphabet();
        SAFE_POINT(0 != activeSequenceAlphabet, "An active sequence alphabet is NULL!", );
        if ((activeSequenceAlphabet->isNucleic()) || (activeSequenceAlphabet->isAmino())) {
            charOccurWidget->show();
        } else {
            // Do not show the characters occurrence for raw alphabet
            charOccurWidget->hide();
        }
    }
}

void SequenceInfo::updateDinuclLayout() {
    ADVSequenceObjectContext *activeSequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(0 != activeSequenceContext, "A sequence context is NULL!", );

    const DNAAlphabet *activeSequenceAlphabet = activeSequenceContext->getAlphabet();
    SAFE_POINT(0 != activeSequenceAlphabet, "An active sequence alphabet is NULL!", );

    const QString alphabetId = activeSequenceAlphabet->getId();
    if ((alphabetId == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()) || (alphabetId == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT())) {
        dinuclWidget->show();
    } else {
        dinuclWidget->hide();
    }
}

void SequenceInfo::updateCodonOccurLayout() {
    ADVSequenceObjectContext *activeSequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(activeSequenceContext != nullptr, "A sequence context is NULL!", );
    codonWidget->setVisible(activeSequenceContext->getAlphabet()->isNucleic());
    aminoAcidWidget->setVisible(activeSequenceContext->getComplementTT() != nullptr && activeSequenceContext->getAminoTT() != nullptr);
}

void SequenceInfo::updateData() {
    updateCommonStatisticsData();
    updateCharactersOccurrenceData();
    updateDinucleotidesOccurrenceData();
    updateCodonsOccurrenceData();
}

void SequenceInfo::updateCommonStatisticsData() {
    if (!getCommonStatisticsCache()->isValid(currentRegions)) {
        launchCalculations(STAT_GROUP_ID);
    } else {
        updateCommonStatisticsData(getCommonStatisticsCache()->getStatistics());
    }
}

namespace {

QString getValue(const QString &value, bool isValid) {
    return isValid ? value : "N/A";
}

}  // namespace

void SequenceInfo::updateCommonStatisticsData(const DNAStatistics &commonStatistics) {
    ADVSequenceWidget *wgt = annotatedDnaView->getActiveSequenceWidget();
    CHECK(wgt != nullptr, );
    ADVSequenceObjectContext *ctx = wgt->getActiveSequenceContext();
    SAFE_POINT(ctx != nullptr, tr("Sequence context is NULL"), );

    const DNAAlphabet *alphabet = ctx->getAlphabet();
    SAFE_POINT(alphabet != nullptr, tr("Sequence alphabet is NULL"), );

    int availableSpace = getAvailableSpace(alphabet->getType());

    const bool isValid = dnaStatisticsTaskRunner.isIdle();

    QString statsInfo = QString("<table cellspacing=%1>").arg(COMMON_STATISTICS_TABLE_CELLSPACING);

    const QString lengthSuffix = alphabet->isNucleic() ? tr("nt") : alphabet->isAmino() ? tr("aa")
                                                                                        : QString();
    statsInfo += formTableRow(CAPTION_SEQ_REGION_LENGTH, getValue(getFormattedLongNumber(commonStatistics.length) + lengthSuffix, isValid), availableSpace);

    if (alphabet->isNucleic()) {
        statsInfo += formTableRow(CAPTION_SEQ_GC_CONTENT, getValue(QString::number(commonStatistics.gcContent, 'f', 2) + "%", isValid), availableSpace);
        statsInfo += formTableRow(CAPTION_SEQ_MELTING_TEMPERATURE, getValue(QString::number(commonStatistics.meltingTemp, 'f', 2) + " &#176;C", isValid), availableSpace);

        const QString ssCaption = alphabet->isRNA() ? CAPTION_SUFFIX_SS_RNA : CAPTION_SUFFIX_SS_DNA;
        statsInfo += QString("<tr><td colspan=2><b>") + tr("%1").arg(ssCaption) + "</b></td></tr>";

        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_NUCL_MOLECULAR_WEIGHT, getValue(QString::number(commonStatistics.ssMolecularWeight, 'f', 2) + tr(" Da"), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_EXTINCTION_COEFFICIENT, getValue(QString::number(commonStatistics.ssExtinctionCoefficient) + tr(" l/(mol * cm)"), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_NMOLE_OD, getValue(QString::number(commonStatistics.ssOd260AmountOfSubstance, 'f', 2), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_MG_OD, getValue(QString::number(commonStatistics.ssOd260Mass, 'f', 2), isValid), availableSpace);

        const QString dsCaption = alphabet->isRNA() ? CAPTION_SUFFIX_DS_RNA : CAPTION_SUFFIX_DS_DNA;
        statsInfo += QString("<tr><td colspan=2><b>") + tr("%1").arg(dsCaption) + "</b></td></tr>";

        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_NUCL_MOLECULAR_WEIGHT, getValue(QString::number(commonStatistics.dsMolecularWeight, 'f', 2) + tr(" Da"), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_EXTINCTION_COEFFICIENT, getValue(QString::number(commonStatistics.dsExtinctionCoefficient) + tr(" l/(mol * cm)"), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_NMOLE_OD, getValue(QString::number(commonStatistics.dsOd260AmountOfSubstance, 'f', 2), isValid), availableSpace);
        statsInfo += formTableRow(QString("&nbsp;").repeated(4) + CAPTION_SEQ_MG_OD, getValue(QString::number(commonStatistics.dsOd260Mass, 'f', 2), isValid), availableSpace);
    } else if (alphabet->isAmino()) {
        statsInfo += formTableRow(CAPTION_SEQ_AMINO_MOLECULAR_WEIGHT, getValue(QString::number(commonStatistics.ssMolecularWeight, 'f', 2), isValid), availableSpace);
        statsInfo += formTableRow(CAPTION_SEQ_ISOELECTIC_POINT, getValue(QString::number(commonStatistics.isoelectricPoint, 'f', 2), isValid), availableSpace);
    }

    statsInfo += "</table>";

    if (statisticLabel->text() != statsInfo) {
        statisticLabel->setText(statsInfo);
    }
}

void SequenceInfo::updateCharactersOccurrenceData() {
    if (!getCharactersOccurrenceCache()->isValid(currentRegions)) {
        launchCalculations(CHAR_OCCUR_GROUP_ID);
    } else {
        updateCharactersOccurrenceData(getCharactersOccurrenceCache()->getStatistics());
    }
}

void SequenceInfo::updateCharactersOccurrenceData(const CharactersOccurrence &charactersOccurrence) {
    const bool isValid = charOccurTaskRunner.isIdle();

    QString charOccurInfo = "<table cellspacing=5>";
    foreach (const CharOccurResult &result, charactersOccurrence) {
        charOccurInfo += "<tr>";
        charOccurInfo += QString("<td><b>") + result.getChar() + QString(":&nbsp;&nbsp;</td>");
        charOccurInfo += "<td>" + getValue(getFormattedLongNumber(result.getNumberOfOccur()), isValid) + "&nbsp;&nbsp;</td>";
        charOccurInfo += "<td>" + getValue(QString::number(result.getPercentage(), 'f', 1) + "%", isValid) + "&nbsp;&nbsp;</td>";
        charOccurInfo += "</tr>";
    }
    charOccurInfo += "</table>";

    if (charOccurLabel->text() != charOccurInfo) {
        charOccurLabel->setText(charOccurInfo);
    }
}

void SequenceInfo::updateDinucleotidesOccurrenceData() {
    if (!getDinucleotidesOccurrenceCache()->isValid(currentRegions)) {
        launchCalculations(DINUCL_OCCUR_GROUP_ID);
    } else {
        updateDinucleotidesOccurrenceData(getDinucleotidesOccurrenceCache()->getStatistics());
    }
}

void SequenceInfo::updateDinucleotidesOccurrenceData(const DinucleotidesOccurrence &dinucleotidesOccurrence) {
    const bool isValid = dinuclTaskRunner.isIdle();

    DinucleotidesOccurrence::const_iterator i = dinucleotidesOccurrence.constBegin();
    DinucleotidesOccurrence::const_iterator end = dinucleotidesOccurrence.constEnd();
    QString dinuclInfo = "<table cellspacing=5>";
    while (i != end) {
        dinuclInfo += "<tr>";
        dinuclInfo += QString("<td><b>") + i.key() + QString(":&nbsp;&nbsp;</td>");
        dinuclInfo += "<td>" + getValue(getFormattedLongNumber(i.value()), isValid) + "&nbsp;&nbsp;</td>";
        dinuclInfo += "</tr>";
        ++i;
    }
    dinuclInfo += "</table>";

    if (dinuclLabel->text() != dinuclInfo) {
        dinuclLabel->setText(dinuclInfo);
    }
}

void SequenceInfo::updateCodonsOccurrenceData() {
    if (!getCodonsOccurrenceCache()->isValid(currentRegions)) {
        launchCalculations(CODON_OCCUR_GROUP_ID);
    } else {
        updateCodonsOccurrenceData(getCodonsOccurrenceCache()->getStatistics());
    }
}

void SequenceInfo::updateCodonsOccurrenceData(const QMap<QByteArray, qint64> &codonStatMap) {
    ADVSequenceObjectContext *activeContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(activeContext != nullptr, "A sequence context is NULL!", );

    DNATranslation *aminoTranslation = activeContext->getAminoTT();
    AnnotationSelection *annotationSelection = activeContext->getAnnotationsSelection();
    DNASequenceSelection *sequenceSelection = activeContext->getSequenceSelection();
    bool isAnnotationSelection = !annotationSelection->isEmpty();
    bool isSequenceSelection = !isAnnotationSelection && !sequenceSelection->isEmpty();
    QString whereInfo = isAnnotationSelection ? (annotationSelection->getAnnotations().size() > 1 ? tr("selected annotations") : tr("selected annotation"))
                                              : (isSequenceSelection ? (sequenceSelection->regions.size() > 1 ? tr("selected regions") : tr("selected region"))
                                                                     : tr("whole sequence"));
    QString frameInfo = isAnnotationSelection ? tr("guided by annotation")
                                              : (isSequenceSelection ? tr("1 direct and 1 complement")
                                                                     : tr("3 direct and 3 complement"));

    QString reportHeaderHtml;
    reportHeaderHtml += "<table cellspacing=5>";
    reportHeaderHtml += "<tr><td>" + tr("Report for:") + "</td><td><i>" + whereInfo + "</i></td></tr>";
    reportHeaderHtml += "<tr><td>" + tr("Frames in report:") + "</td><td><i>" + frameInfo + "</i></td></tr>";
    reportHeaderHtml += "</table>";

    QString codonsHtml = reportHeaderHtml;
    QString aminoAcidsHtml = reportHeaderHtml;

    if (codonStatMap.isEmpty()) {
        QString selectionIsTooSmallHtml = "<tr><td>" + tr("Selection is too small") + "</td></tr>";
        codonsHtml += selectionIsTooSmallHtml;
        aminoAcidsHtml += selectionIsTooSmallHtml;
    } else {
        bool isFinal = codonTaskRunner.isIdle();
        // Prepare codons report & convert raw codons into amino acids.
        QList<QByteArray> codons = codonStatMap.keys();
        QHash<char, qint64> countPerAminoAcid;
        bool tableColumnIndex = 0;  // Report is written using 2 columns.
        for (const QByteArray &codon : qAsConst(codons)) {
            codonsHtml += tableColumnIndex == 0 ? "<tr>" : "";
            codonsHtml += QString("<td><b>") + QString::fromLatin1(codon) + QString(":&nbsp;&nbsp;</b></td>");
            qint64 count = codonStatMap.value(codon);
            codonsHtml += "<td>" + getValue(getFormattedLongNumber(count), isFinal) + "&nbsp;&nbsp;</td>";
            codonsHtml += tableColumnIndex == 1 ? "</tr>" : "";
            tableColumnIndex = (tableColumnIndex + 1) % 2;
            if (aminoTranslation != nullptr) {
                char acidChar = 0;
                aminoTranslation->translate(codon.constData(), 3, &acidChar, 1);
                if (acidChar != 0) {
                    countPerAminoAcid[acidChar] += count;
                }
            }
        }

        // Prepare amino acids report.
        QList<CharOccurResult> aminoAcidList;
        for (auto it = countPerAminoAcid.begin(); it != countPerAminoAcid.end(); it++) {
            char aminoAcid = it.key();
            qint64 count = it.value();
            aminoAcidList << CharOccurResult(aminoAcid, count, 0);
        }
        std::sort(aminoAcidList.begin(), aminoAcidList.end(), [](const CharOccurResult &c1, CharOccurResult &c2) {
            // Reversed order: most frequent is first, if both counts are equal sort by character.
            return c1.getNumberOfOccur() != c2.getNumberOfOccur() ? c1.getNumberOfOccur() > c2.getNumberOfOccur() : c1.getChar() < c2.getChar();
        });
        for (const CharOccurResult &aminoAcid : qAsConst(aminoAcidList)) {
            aminoAcidsHtml += "<tr>";
            aminoAcidsHtml += QString("<td><b>") + aminoAcid.getChar() + QString(":&nbsp;&nbsp;</b></td>");
            aminoAcidsHtml += "<td>" + getValue(getFormattedLongNumber(aminoAcid.getNumberOfOccur()), isFinal) + "&nbsp;&nbsp;</td>";
            aminoAcidsHtml += "</tr>";
        }
    }
    codonsHtml += "</table>";
    aminoAcidsHtml += "</table>";

    if (codonLabel->text() != codonsHtml) {
        codonLabel->setText(codonsHtml);
    }

    if (aminoAcidLabel->text() != aminoAcidsHtml) {
        aminoAcidLabel->setText(aminoAcidsHtml);
    }
}

void SequenceInfo::connectSlotsForSeqContext(ADVSequenceObjectContext *seqContext) {
    SAFE_POINT(seqContext, "A sequence context is NULL!", );

    connect(seqContext->getSequenceSelection(),
            SIGNAL(si_selectionChanged(LRegionsSelection *, const QVector<U2Region> &, const QVector<U2Region> &)),
            SLOT(sl_onSelectionChanged(LRegionsSelection *, const QVector<U2Region> &, const QVector<U2Region> &)));

    connect(seqContext->getAnnotationsSelection(),
            SIGNAL(si_selectionChanged(AnnotationSelection *, const QList<Annotation *> &, const QList<Annotation *> &)),
            SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection *, const QList<Annotation *> &, const QList<Annotation *> &)));

    connect(seqContext, SIGNAL(si_aminoTranslationChanged()), SLOT(sl_onAminoTranslationChanged()));
    connect(seqContext->getSequenceObject(), SIGNAL(si_sequenceChanged()), SLOT(sl_onSequenceModified()));
}

void SequenceInfo::connectSlots() {
    QList<ADVSequenceObjectContext *> seqContexts = annotatedDnaView->getSequenceContexts();
    SAFE_POINT(!seqContexts.empty(), "AnnotatedDNAView has no sequences contexts!", );

    // A sequence has been selected in the Sequence View
    connect(annotatedDnaView, SIGNAL(si_activeSequenceWidgetChanged(ADVSequenceWidget *, ADVSequenceWidget *)), this, SLOT(sl_onActiveSequenceChanged(ADVSequenceWidget *, ADVSequenceWidget *)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext *)), this, SLOT(sl_onSequenceModified()));

    // A user has selected a sequence region
    foreach (ADVSequenceObjectContext *seqContext, seqContexts) {
        connectSlotsForSeqContext(seqContext);
    }

    // A sequence object has been added
    connect(annotatedDnaView, SIGNAL(si_sequenceAdded(ADVSequenceObjectContext *)), SLOT(sl_onSequenceAdded(ADVSequenceObjectContext *)));

    // Calculations have been finished
    connect(&charOccurTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateCharOccurData()));
    connect(&dinuclTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateDinuclData()));
    connect(&dnaStatisticsTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateStatData()));
    connect(&codonTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateCodonOccurData()));

    // A subgroup has been opened/closed
    connect(charOccurWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
    connect(dinuclWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
    connect(codonWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
    connect(aminoAcidWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
}

void SequenceInfo::sl_onSelectionChanged(LRegionsSelection *,
                                         const QVector<U2Region> & /*added*/,
                                         const QVector<U2Region> & /*removed*/) {
    getCodonsOccurrenceCache()->sl_invalidate();
    updateCurrentRegions();
    updateData();
}

void SequenceInfo::sl_onAnnotationSelectionChanged(AnnotationSelection *, const QList<Annotation *> &, const QList<Annotation *> &) {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getActiveSequenceContext();
    if (sequenceContext == nullptr) {
        return;  // Sequence context may be null when ADV is being destroyed.
    }
    getCodonsOccurrenceCache()->sl_invalidate();
    updateCurrentRegions();
    updateData();
}

void SequenceInfo::sl_onAminoTranslationChanged() {
    getCodonsOccurrenceCache()->sl_invalidate();
    updateCurrentRegions();
    updateData();
}

void SequenceInfo::sl_onSequenceModified() {
    updateCurrentRegions();
    updateData();
}

void SequenceInfo::sl_onActiveSequenceChanged(ADVSequenceWidget * /*oldSequenceWidget*/, ADVSequenceWidget *newSequenceWidget) {
    if (newSequenceWidget != nullptr) {  // i.e. the sequence has been deleted
        updateLayout();
        updateCurrentRegions();
        updateData();
    }
}

void SequenceInfo::sl_onSequenceAdded(ADVSequenceObjectContext *seqContext) {
    connectSlotsForSeqContext(seqContext);
}

void SequenceInfo::sl_subgroupStateChanged(const QString &subgroupId) {
    if (subgroupId == STAT_GROUP_ID) {
        updateCommonStatisticsData();
    } else if (subgroupId == CHAR_OCCUR_GROUP_ID) {
        updateCharactersOccurrenceData();
    } else if (subgroupId == DINUCL_OCCUR_GROUP_ID) {
        updateDinucleotidesOccurrenceData();
    } else if (subgroupId == CODON_OCCUR_GROUP_ID || subgroupId == AMINO_ACID_OCCUR_GROUP_ID) {
        updateCodonsOccurrenceData();
    }
}

bool SequenceInfo::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::Resize && object == statisticLabel) {
        updateCommonStatisticsData();
    }
    return false;
}

void SequenceInfo::updateCurrentRegions() {
    ADVSequenceObjectContext *seqContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(0 != seqContext, "A sequence context is NULL!", );

    DNASequenceSelection *selection = seqContext->getSequenceSelection();

    QVector<U2Region> selectedRegions = selection->getSelectedRegions();
    if (!selectedRegions.empty()) {
        currentRegions = selectedRegions;
    } else {
        currentRegions.clear();
        currentRegions << U2Region(0, seqContext->getSequenceLength());
    }
}

void SequenceInfo::launchCalculations(const QString &subgroupId) {
    // Launch the statistics, characters and dinucleotides calculation tasks,
    // if corresponding groups are present and opened
    ADVSequenceObjectContext *activeContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(activeContext != nullptr, "A sequence context is NULL!", );

    U2SequenceObject *seqObj = activeContext->getSequenceObject();
    U2EntityRef seqRef = seqObj->getSequenceRef();
    const DNAAlphabet *alphabet = activeContext->getAlphabet();

    if (subgroupId.isEmpty() || subgroupId == CHAR_OCCUR_GROUP_ID) {
        if (!charOccurWidget->isHidden() && charOccurWidget->isSubgroupOpened()) {
            charOccurWidget->showProgress();
            charOccurTaskRunner.run(new CharOccurTask(alphabet, seqRef, currentRegions));
            getCharactersOccurrenceCache()->sl_invalidate();
            updateCharactersOccurrenceData(getCharactersOccurrenceCache()->getStatistics());
        }
    }

    if (subgroupId.isEmpty() || subgroupId == DINUCL_OCCUR_GROUP_ID) {
        if (!dinuclWidget->isHidden() && dinuclWidget->isSubgroupOpened()) {
            dinuclWidget->showProgress();
            dinuclTaskRunner.run(new DinuclOccurTask(alphabet, seqRef, currentRegions));
            getDinucleotidesOccurrenceCache()->sl_invalidate();
            updateDinucleotidesOccurrenceData(getDinucleotidesOccurrenceCache()->getStatistics());
        }
    }

    if (subgroupId.isEmpty() || subgroupId == CODON_OCCUR_GROUP_ID || subgroupId == AMINO_ACID_OCCUR_GROUP_ID) {
        bool isCodonReportOpened = codonWidget->isVisible() && codonWidget->isSubgroupOpened();
        bool isAminoAcidReportOpened = aminoAcidWidget->isVisible() && aminoAcidWidget->isSubgroupOpened();
        if (isCodonReportOpened || isAminoAcidReportOpened) {
            if (isCodonReportOpened) {
                codonWidget->showProgress();
            }
            if (isAminoAcidReportOpened) {
                aminoAcidWidget->showProgress();
            }
            DNATranslation *complementTranslation = activeContext->getComplementTT();
            AnnotationSelection *annotationSelection = activeContext->getAnnotationsSelection();
            DNASequenceSelection *sequenceSelection = activeContext->getSequenceSelection();
            auto task = !annotationSelection->isEmpty()
                            ? new CodonOccurTask(complementTranslation, seqRef, annotationSelection->getAnnotations())
                            : (!sequenceSelection->isEmpty()
                                   ? new CodonOccurTask(complementTranslation, seqRef, sequenceSelection->getSelectedRegions())
                                   : new CodonOccurTask(complementTranslation, seqRef));
            codonTaskRunner.run(task);
            getCodonsOccurrenceCache()->sl_invalidate();
            updateCodonsOccurrenceData(getCodonsOccurrenceCache()->getStatistics());
        }
    }

    if (subgroupId.isEmpty() || subgroupId == STAT_GROUP_ID) {
        if ((!statsWidget->isHidden()) && (statsWidget->isSubgroupOpened())) {
            statsWidget->showProgress();
            dnaStatisticsTaskRunner.run(new DNAStatisticsTask(alphabet, seqRef, currentRegions));
            getCommonStatisticsCache()->sl_invalidate();
            updateCommonStatisticsData(getCommonStatisticsCache()->getStatistics());
        }
    }
}

int SequenceInfo::getAvailableSpace(DNAAlphabetType alphabetType) const {
    QStringList captions;
    switch (alphabetType) {
        case DNAAlphabet_NUCL:
            captions << CAPTION_SEQ_REGION_LENGTH
                     << CAPTION_SEQ_GC_CONTENT
                     << CAPTION_SEQ_MELTING_TEMPERATURE
                     << QString("    ") + CAPTION_SEQ_NUCL_MOLECULAR_WEIGHT
                     << QString("    ") + CAPTION_SEQ_EXTINCTION_COEFFICIENT;
            // Two captions are ignored because of HTML tags within them
            //                 << CAPTION_SEQ_NMOLE_OD
            //                 << CAPTION_SEQ_MG_OD;
            break;
        case DNAAlphabet_AMINO:
            captions << CAPTION_SEQ_REGION_LENGTH
                     << CAPTION_SEQ_AMINO_MOLECULAR_WEIGHT
                     << CAPTION_SEQ_ISOELECTIC_POINT;
            break;
        default:
            captions << CAPTION_SEQ_REGION_LENGTH;
            break;
    }

    QFontMetrics fontMetrics(statisticLabel->font());

    int availableSize = INT_MAX;
    foreach (const QString &caption, captions) {
        availableSize = qMin(availableSize, statisticLabel->width() - fontMetrics.boundingRect(caption).width() - 3 * COMMON_STATISTICS_TABLE_CELLSPACING);
    }

    return availableSize;
}

void SequenceInfo::sl_updateCharOccurData() {
    charOccurWidget->hideProgress();
    getCharactersOccurrenceCache()->setStatistics(charOccurTaskRunner.getResult(), currentRegions);
    updateCharactersOccurrenceData(getCharactersOccurrenceCache()->getStatistics());
}

void SequenceInfo::sl_updateDinuclData() {
    dinuclWidget->hideProgress();
    getDinucleotidesOccurrenceCache()->setStatistics(dinuclTaskRunner.getResult(), currentRegions);
    updateDinucleotidesOccurrenceData(getDinucleotidesOccurrenceCache()->getStatistics());
}

void SequenceInfo::sl_updateStatData() {
    statsWidget->hideProgress();
    getCommonStatisticsCache()->setStatistics(dnaStatisticsTaskRunner.getResult(), currentRegions);
    updateCommonStatisticsData(getCommonStatisticsCache()->getStatistics());
}

void SequenceInfo::sl_updateCodonOccurData() {
    codonWidget->hideProgress();
    aminoAcidWidget->hideProgress();
    getCodonsOccurrenceCache()->setStatistics(codonTaskRunner.getResult(), currentRegions);
    updateCodonsOccurrenceData(getCodonsOccurrenceCache()->getStatistics());
}

QString SequenceInfo::formTableRow(const QString &caption, const QString &value, int availableSpace) const {
    QString result;

    QFontMetrics metrics = statisticLabel->fontMetrics();
    result = "<tr><td>" + tr("%1").arg(caption) + "</td><td>" + metrics.elidedText(value, Qt::ElideRight, availableSpace) + "</td></tr>";
    return result;
}

StatisticsCache<DNAStatistics> *SequenceInfo::getCommonStatisticsCache() const {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(sequenceContext != nullptr, "A sequence context is NULL!", nullptr);
    return sequenceContext->getCommonStatisticsCache();
}

StatisticsCache<CharactersOccurrence> *SequenceInfo::getCharactersOccurrenceCache() const {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(sequenceContext != nullptr, "A sequence context is NULL!", nullptr);
    return sequenceContext->getCharactersOccurrenceCache();
}

StatisticsCache<DinucleotidesOccurrence> *SequenceInfo::getDinucleotidesOccurrenceCache() const {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(sequenceContext != nullptr, "A sequence context is NULL!", nullptr);
    return sequenceContext->getDinucleotidesOccurrenceCache();
}

StatisticsCache<QMap<QByteArray, qint64>> *SequenceInfo::getCodonsOccurrenceCache() const {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getActiveSequenceContext();
    SAFE_POINT(sequenceContext != nullptr, "A sequence context is NULL!", nullptr);
    return sequenceContext->getCodonsOccurrenceCache();
}

}  // namespace U2
