/* =========================================================================
 *  Copyright 2017-2021 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ========================================================================= */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "libfci_cli_common.h"
#include "libfci_cli_def_help.h"
#include "libfci_cli_parser.h"
#include "fci_ep.h"

/* ==== TYPEDEFS & DATA ==================================================== */

FCI_CLIENT* cli_p_cl = NULL;

/* ==== PUBLIC FUNCTIONS =================================================== */

void cli_print_error(int rtncode, const char* p_txt_err, ...)
{
    assert(NULL != p_txt_err);

    printf("ERROR (%d): ", rtncode);
    
    va_list args;
    va_start(args, p_txt_err);
    vprintf(p_txt_err, args);
    va_end(args);
    
    printf("\n");
}

int main(int argc, char* argv[])
{
    #if !defined(NDEBUG)
        #warning "DEBUG build"
        printf("\nWARNING: DEBUG build\n");
    #endif
    
    
    int rtn = CLI_ERR;
    
    printf("DISCLAIMER: This is a DEMO application. It is not part of the production code deliverables.\n");
    
    if (1 >= argc)
    {
        cli_print_app_version();
        cli_print_help(0);
    }
    
    rtn = fci_ep_open_in_cmd_mode(&cli_p_cl);
    if (CLI_OK != rtn)
    {
        cli_print_error(rtn, "FCI endpoint failed to open.");
    }
    else
    {
        rtn = cli_parse_and_execute(argv, argc);
    }
    
    /* close FCI (do not hide behind rtn check) */
    if (NULL != cli_p_cl)
    {
        const int rtn_close = fci_ep_close(cli_p_cl);
        rtn = ((CLI_OK == rtn) ? (rtn_close) : (rtn));
        if (CLI_OK != rtn_close)
        {
            cli_print_error(rtn_close, "FCI endpoint failed to close.");
        }
    }
    
    return (rtn);
}

/* ========================================================================= */