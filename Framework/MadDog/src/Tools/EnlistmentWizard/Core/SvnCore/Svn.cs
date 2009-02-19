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
        /// <param name="account"></param>
        /// <param name="password"></param>
        /// <param name="destDir">Destination directory on the local disk</param>
        /// <returns>Run successfully or not</returns>
        public static Boolean SvnCheckoutWorkspace(String account,String password,String destDir)
        {
            Process p = System.Diagnostics.Process.Start(new ProcessStartInfo( @"svn.exe",
                String.Format("checkout {0} {1} --username {2} --password {3}", SERVER_PROJ_ROOT, destDir,account,password)));       
            p.WaitForExit();   
            //Wait for exit
            return (p.ExitCode == 0);
        }
    }
}
