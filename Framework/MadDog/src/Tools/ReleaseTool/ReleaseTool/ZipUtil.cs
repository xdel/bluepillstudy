using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ICSharpCode.SharpZipLib.Zip;
using System.IO;

namespace ReleaseTool
{
    public class ZipUtil
    {
        /// <summary>
        /// Create a zip file.
        /// </summary>
        /// <param name="fileNames"></param>
        /// <param name="outputZipFile">Create a zipfile. it will override the existed files.</param>
        /// <returns></returns>
        public Boolean ZipFile(String[] fileNames,String outputZipFile)
        {
            try
		    {
			    // 'using' statements gaurantee the stream is closed properly which is a big source
			    // of problems otherwise.  Its exception safe as well which is great.
                using (ZipOutputStream s = new ZipOutputStream(File.Create(outputZipFile)))
                { 			
				    s.SetLevel(9); // 0 - store only to 9 - means best compression
    		
				    byte[] buffer = new byte[4096];

                    foreach (String file in fileNames)
                    {	
					    // Using GetFileName makes the result compatible with XP
					    // as the resulting path is not absolute.
					    ZipEntry entry = new ZipEntry(Path.GetFileName(file));
    					
					    // Setup the entry data as required.
    					
					    // Crc and size are handled by the library for seakable streams
					    // so no need to do them here.

					    // Could also use the last write time or similar for the file.
					    entry.DateTime = DateTime.Now;
					    s.PutNextEntry(entry);
    					
					    using ( FileStream fs = File.OpenRead(file) ) {
    		
						    // Using a fixed size buffer here makes no noticeable difference for output
						    // but keeps a lid on memory usage.
						    int sourceBytes;
						    do {
							    sourceBytes = fs.Read(buffer, 0, buffer.Length);
							    s.Write(buffer, 0, sourceBytes);
						    } while ( sourceBytes > 0 );
					    }
				    }
    				
				    // Finish/Close arent needed strictly as the using statement does this automatically
    				
				    // Finish is important to ensure trailing information for a Zip file is appended.  Without this
				    // the created file would be invalid.
				    s.Finish();
    				
				    // Close is important to wrap things up and unlock the file.
				    s.Close();
			    }
		    }
		    catch(Exception ex)
		    {
                return false;
		    }
            return true;
        }
    }
}
