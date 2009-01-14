using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using IWshRuntimeLibrary;

namespace Tools.EnlistmentWizard.Core.Util
{
    public class ShortcutHelper
    {
        public static void CreateShortcut(String path, String targetPath, String args, String workingDirectory)
        {
            WshShell shell = new WshShell();
            IWshShortcut shortcut = (IWshShortcut)shell.CreateShortcut(path);
            shortcut.TargetPath = targetPath;
            shortcut.Arguments = args;
            shortcut.WorkingDirectory = workingDirectory;
            shortcut.WindowStyle = 1;
            shortcut.Description = "Razzle For Laboratory VT Project";
            shortcut.IconLocation = System.Environment.SystemDirectory + "\\" + "shell32.dll, 165";
            shortcut.Save();
        }
    }
}
