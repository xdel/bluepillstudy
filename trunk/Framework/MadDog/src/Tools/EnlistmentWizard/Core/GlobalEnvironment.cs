using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using Microsoft.Win32;

namespace Tools.EnlistmentWizard.Core
{
    public class GlobalEnvironment
    {
        private static int razzleCount = 0;
        /// <summary>
        /// The WINDDK_HOME which will be append in System variables 
        /// </summary>
        public const String NAME_DEAULT_WINDDK_HOME = "DDKHome";
        public const String NAME_RAZZLE_COUNT = "RazzleCount";

        public static String DDKHome
        {
            get
            {
                RegistryKey myEnv = GetGlobalEnv();
                String value = (String)myEnv.GetValue(GlobalEnvironment.NAME_DEAULT_WINDDK_HOME);
                return value;
            }
            set
            {
                RegistryKey myEnv = GetGlobalEnv();
                myEnv.SetValue(GlobalEnvironment.NAME_DEAULT_WINDDK_HOME, value);
            }
        }
        public static int RazzleCount
        {
            get
            {
                RegistryKey myEnv = GetGlobalEnv();
                String razzleCountStr = (String)myEnv.GetValue(GlobalEnvironment.NAME_RAZZLE_COUNT);
                if (razzleCountStr != null)
                {
                    razzleCount = Int32.Parse(razzleCountStr);
                }
                
                return razzleCount;
            }
        }
        public static void IncreaseRazzleCount()
        {
            RegistryKey myEnv = GetGlobalEnv();
            myEnv.SetValue(GlobalEnvironment.NAME_RAZZLE_COUNT, (++razzleCount).ToString());
        }
        private static RegistryKey GetGlobalEnv()
        {
            RegistryKey rk = Registry.LocalMachine;

            RegistryKey myEnv = rk.OpenSubKey("System").OpenSubKey("ControlSet001").OpenSubKey("Control").OpenSubKey("Session Manager").
                OpenSubKey("Environment",true);
            
            return myEnv;
        }
    }
}
