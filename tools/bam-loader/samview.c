/* ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#include <kapp/args.h>
#include <kapp/main.h>
#include <klib/log.h>
#include <kfs/file.h>

#include <stdlib.h>
#include <stdio.h>

#include "bam.h"

#include <klib/rc.h>

static void writeHeader(BAM_File const *const bam)
{
    char const *header = NULL;
    size_t hsize = 0;

    BAM_FileGetHeaderText(bam, &header, &hsize);
    fwrite(header, 1, hsize, stdout);
}

static rc_t writeSAM(BAM_Alignment const *const rec)
{
    static char buffer[64*1024];
    size_t actsize = 0;
    rc_t const rc = BAM_AlignmentFormatSAM(rec, &actsize, sizeof(buffer), buffer);

    if (rc == 0)
        fwrite(buffer, 1, actsize, stdout);

    return rc;
}

static
void samview(char const path[])
{
    BAM_File const *bam = NULL;
    rc_t rc = BAM_FileMake(&bam, NULL, NULL, path);

    if (rc == 0) {
        BAM_Alignment const *rec = NULL;

        writeHeader(bam);
        while ((rc = BAM_FileRead2(bam, &rec)) == 0) {
            rc_t const rc2 = writeSAM(rec);
            BAM_AlignmentRelease(rec);
            if (rc2)
                break;
        }
        BAM_FileRelease(bam);
        if (GetRCObject(rc) == rcRow && GetRCState(rc) == rcNotFound)
            rc = 0;
    }
    if (rc)
        LOGERR(klogWarn, rc, "Final RC");
}

rc_t CC UsageSummary(char const *name)
{
    return 0;
}

rc_t CC Usage(Args const *args)
{
    return 0;
}

ver_t CC KAppVersion(void)
{
    return 0;
}

rc_t CC KMain(int argc, char *argv[])
{
    if (argc == 1) {
        samview("/dev/stdin");
        return 0;
    }
    while (--argc) {
        samview(*++argv);
    }
    return 0;
}
