using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Tools.EnlistmentWizard.Core
{
    public class TemplateStrings
    {
        /// <summary>
        /// This String is in Razzle.bat
        /// set ENLISTMENT_PROJ_ROOT=%%ENLISTMENT_PROJ_ROOT%%
        /// </summary>
        public const String LOCAL_ENLISTMENT_PROJ_ROOT_STRING = "%%ENLISTMENT_PROJ_ROOT%%";

        /// <summary>
        /// This String is in Razzle.bat
        /// set RAZZLE_INDEX=%%RAZZLE_INDEX%%
        /// </summary>
        public const String LOCAL_RAZZLE_INDEX = "%%RAZZLE_INDEX%%";
    }
}
