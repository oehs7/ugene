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

#include "MultipleAlignmentRow.h"

#include <U2Core/U2SafePoints.h>

#include "MultipleAlignment.h"

namespace U2 {

MultipleAlignmentRow::MultipleAlignmentRow(MultipleAlignmentRowData *ma)
    : maRowData(ma) {
}

MultipleAlignmentRow::~MultipleAlignmentRow() {
}

MultipleAlignmentRowData *MultipleAlignmentRow::data() const {
    return maRowData.data();
}

MultipleAlignmentRowData &MultipleAlignmentRow::operator*() {
    return *maRowData;
}

const MultipleAlignmentRowData &MultipleAlignmentRow::operator*() const {
    return *maRowData;
}

MultipleAlignmentRowData *MultipleAlignmentRow::operator->() {
    return maRowData.data();
}

const MultipleAlignmentRowData *MultipleAlignmentRow::operator->() const {
    return maRowData.data();
}

MultipleAlignmentRowData::MultipleAlignmentRowData() {
}

MultipleAlignmentRowData::MultipleAlignmentRowData(const DNASequence &sequence, const QList<U2MsaGap> &gaps)
    : sequence(sequence),
      gaps(gaps) {
}

int MultipleAlignmentRowData::getUngappedPosition(int pos) const {
    return MsaRowUtils::getUngappedPosition(gaps, sequence.length(), pos);
}

DNASequence MultipleAlignmentRowData::getUngappedSequence() const {
    return sequence;
}

U2Region MultipleAlignmentRowData::getGapped(const U2Region &region) {
    return MsaRowUtils::getGappedRegion(gaps, region);
}

bool MultipleAlignmentRowData::isTrailingOrLeadingGap(qint64 position) const {
    CHECK(isGap(position), false);
    return position < getCoreStart() || position > getCoreEnd() - 1;
}

U2Region MultipleAlignmentRowData::getCoreRegion() const {
    return U2Region(getCoreStart(), getCoreLength());
}

U2Region MultipleAlignmentRowData::getUngappedRegion(const U2Region &gappedRegion) const {
    if (gappedRegion == U2Region(0, 0)) {
        return gappedRegion;
    }
    U2Region noTrailingGapsRegion(gappedRegion);

    if (noTrailingGapsRegion.endPos() > getRowLengthWithoutTrailing()) {
        noTrailingGapsRegion.length = getRowLengthWithoutTrailing() - noTrailingGapsRegion.startPos;
    }
    return MsaRowUtils::getUngappedRegion(gaps, noTrailingGapsRegion);
}

/* Compares sequences of 2 rows ignoring gaps. */
bool MultipleAlignmentRowData::isEqualsIgnoreGaps(const MultipleAlignmentRowData *row1, const MultipleAlignmentRowData *row2) {
    SAFE_POINT(row1 != nullptr && row2 != nullptr, "One of the rows is nullptr!", false);
    if (row1 == row2) {
        return true;
    }
    if (row1->getUngappedLength() != row2->getUngappedLength()) {
        return false;
    }
    return row1->getUngappedSequence().seq == row2->getUngappedSequence().seq;
}

QByteArray MultipleAlignmentRowData::getSequenceWithGaps(bool keepLeadingGaps, bool keepTrailingGaps) const {
    QByteArray bytes = sequence.constSequence();
    int beginningOffset = 0;

    if (gaps.isEmpty()) {
        return bytes;
    }

    for (int i = 0; i < gaps.size(); ++i) {
        QByteArray gapsBytes;
        if (!keepLeadingGaps && (0 == gaps[i].offset)) {
            beginningOffset = gaps[i].gap;
            continue;
        }

        gapsBytes.fill(U2Msa::GAP_CHAR, gaps[i].gap);
        bytes.insert(gaps[i].offset - beginningOffset, gapsBytes);
    }
    MultipleAlignmentData *alignment = getMultipleAlignmentData();
    SAFE_POINT(alignment != nullptr, "Parent MAlignment is NULL", QByteArray());
    if (keepTrailingGaps && bytes.size() < alignment->getLength()) {
        QByteArray gapsBytes;
        gapsBytes.fill(U2Msa::GAP_CHAR, alignment->getLength() - bytes.size());
        bytes.append(gapsBytes);
    }

    return bytes;
}

}  // namespace U2
