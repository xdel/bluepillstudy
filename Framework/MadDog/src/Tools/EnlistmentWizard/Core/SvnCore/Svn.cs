using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Diagnostics;

namespace Core.Svn
{
    public class SvnService
    {
        private const String SERVER_PROJ_ROOT ="https://bluepillstudy.googlecode.com/svn/trunk/Framework";
        
        /// <summary>
        /// Checkout the whole workspace from the server
        /// </summary>
        /// <param name="destDir">Destination directory on the local disk</param>
        /// <returns>Run successfully or not</returns>
        public static Boolean SvnCheckoutWorkspace(String destDir)
        {
            String svnCmdPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
            Process p = System.Diagnostics.Process.Start(new ProcessStartInfo(svnCmdPath + @"\svn.exe",
                String.Format("checkout {0} {1}", SERVER_PROJ_ROOT, destDir)));       
            p.WaitForExit();   
            //Wait for exit
            return (p.ExitCode == 0);
        }
    }
}
