using System;
using System.Collections.Generic;
using System.Text;

namespace Controller
{
    public class GameConfig
    {
        public GameConfig( string InGame, string InPlatform, string InConfiguration )
        {
            GameName = InGame + "Game";
            Platform = InPlatform;
            Configuration = InConfiguration;
        }

        public GameConfig( int Count, GameConfig Game )
        {
            switch( Count )
            {
                case 2:
                    GameName = "Some Games";
                    break;

                case 3:
                    GameName = "All Games";
                    break;
            }
            Platform = Game.Platform;
            Configuration = Game.Configuration;
        }

        public string GetConfiguration()
        {
            if( Platform.ToLower() == "xenon" )
            {
                return ( "Xe" + Configuration );
            }
            else if( Platform.ToLower() == "ps3" )
            {
                return ( "PS3" + Configuration );
            }

            return ( Configuration );
        }

        public string[] GetExecutableNames()
        {
            string[] Configs = new string[2] { "", "" };

            if( Platform.ToLower() == "pc" )
            {
                if( Configuration.ToLower() == "release" )
                {
                    Configs[0] = "Binaries/" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "releaseltcg" )
                {
                    Configs[0] = "Binaries/LTCG-" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "debug" )
                {
                    Configs[0] = "Binaries/DEBUG-" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "release-g4wlive" )
                {
                    Configs[0] = "Binaries/" + GameName + "-G4WLive.exe";
                }
                else if( Configuration.ToLower() == "releaseltcg-g4wlive" )
                {
                    Configs[0] = "Binaries/" + GameName + "LTCG-G4WLive.exe";
                }
                else if( Configuration.ToLower() == "releaseshippingpc" )
                {
                    if( GameName.ToLower() == "utgame" )
                    {
                        Configs[0] = "Binaries/UT3.exe";
                    }
                    else
                    {
                        Configs[0] = "Binaries/" + GameName + "-ShippingPC.exe";
                    }
                }
            }
            else if( Platform.ToLower() == "xenon" )
            {
                Configs[0] = GameName + "-" + GetConfiguration() + ".exe";
                Configs[1] = GameName + "-" + GetConfiguration() + ".xex";
            }
            else if( Platform.ToLower() == "ps3" )
            {
                Configs[0] = "Binaries/" + GameName.ToUpper() + "-" + GetConfiguration() + ".elf";
                Configs[1] = "Binaries/" + GameName.ToUpper() + "-" + GetConfiguration() + ".xelf";
            }

            return ( Configs );
        }

        public string GetExeName()
        {
            string ComName = "";

            if( Configuration.ToLower() == "release-g4wlive" || Configuration.ToLower() == "releaseltcg-g4wlive" )
            {
                ComName = "Binaries/" + GameName + "-G4WLive.exe";
            }
            else
            {
                ComName = "Binaries/" + GameName + ".exe";
            }

            return ( ComName );
        }

        public string GetSymbolFileName()
        {
            if( Platform.ToLower() == "pc" )
            {
                if( Configuration.ToLower() == "release" )
                {
                    return ( "Binaries/Lib/" + GetConfiguration() + "/" + GameName + ".pdb" );
                }
                else if( Configuration.ToLower() == "releaseltcg" )
                {
                    return ( "Binaries/Lib/" + GetConfiguration() + "/LTCG-" + GameName + ".pdb" );
                }
                else if( Configuration.ToLower() == "debug" )
                {
                    return ( "Binaries/Lib/" + GetConfiguration() + "/DEBUG-" + GameName + ".pdb" );
                }
                else if( Configuration.ToLower() == "release-g4wlive" )
                {
                    return( "Binaries/Lib/" + GetConfiguration() + "/" + GameName + "-G4WLive.pdb" );
                }
                else if( Configuration.ToLower() == "releaseltcg-g4wlive" )
                {
                    return ( "Binaries/Lib/" + GetConfiguration() + "/" + GameName + "LTCG-G4WLive.pdb" );
                }
                else if( Configuration.ToLower() == "releaseshippingpc" )
                {
                    if( GameName.ToLower() == "utgame" )
                    {
                        return ( "Binaries/Lib/" + GetConfiguration() + "/UT3.pdb" );
                    }
                    else
                    {
                        return ( "Binaries/Lib/" + GetConfiguration() + "/" + GameName + "-ShippingPC.pdb" );
                    }
                }
            }
            else if( Platform.ToLower() == "xenon" )
            {
                return ( "Binaries/Xenon/lib/" + GetConfiguration() + "/" + GameName + "-" + GetConfiguration() + ".pdb" );
            }
            else if( Platform.ToLower() == "ps3" )
            {
                return ( "" );
            }

            return ( "" );
        }

        private string GetShaderName( string ShaderType )
        {
            string ShaderName = GameName + "/Content/" + ShaderType + "ShaderCache";

            if( Platform.ToLower() == "pc" )
            {
                ShaderName += "-PC-D3D-SM3.upk";
            }
            else if( Platform.ToLower() == "xenon" )
            {
                ShaderName += "-Xbox360.upk";
            }
            else if( Platform.ToLower() == "ps3" )
            {
                ShaderName += "-PS3.upk";
            }
            else if( Platform.ToLower() == "pc_sm2" )
            {
                ShaderName += "-PC-D3D-SM2.upk";
            }
            else if( Platform.ToLower() == "pc_sm4" )
            {
                ShaderName += "-PC-D3D-SM4.upk";
            }

            return ( ShaderName );
        }

        public string GetRefShaderName()
        {
            return ( GetShaderName( "Ref" ) );
        }

        public string GetLocalShaderName()
        {
            return ( GetShaderName( "Local" ) );
        }

        public string GetConfigFolderName()
        {
            string Folder = GameName + "/config";
            return ( Folder );
        }

        public string GetCookedFolderName()
        {
            string Folder = GameName + "/Cooked" + Platform;
            return ( Folder );
        }

        public string GetJobsFolderName()
        {
            string Folder = GameName + "/Jobs-" + Platform;
            return ( Folder );
        }
        
        public string GetDialogFileName( string Language, string RootName )
        {
            string DialogName;

            if( Language == "INT" )
            {
                DialogName = GameName + "/Content/Sounds/" + Language + "/" + RootName + ".upk";
            }
            else
            {
                DialogName = GameName + "/Content/Sounds/" + Language + "/" + RootName + "_" + Language + ".upk";
            }
            return ( DialogName );
        }

        public string GetFontFileName( string Language, string RootName )
        {
            string FontName;

            if( Language == "INT" )
            {
                FontName = GameName + "/Content/" + RootName + ".upk";
            }
            else
            {
                FontName = GameName + "/Content/" + RootName + "_" + Language + ".upk";
            }
            return ( FontName );
        }

        public string GetPackageFileName( string Language, string RootName )
        {
            string PackageName;

            if( Language == "INT" )
            {
                PackageName = RootName + ".upk";
            }
            else
            {
                PackageName = RootName + "_" + Language + ".upk";
            }
            return ( PackageName );
        }
        
        public string GetTitle()
        {
            return ( "UnrealEngine3-" + Platform );
        }

        override public string ToString()
        {
            return ( GameName + " (" + Platform + " - " + Configuration + ")" ); 
        }

        public bool Similar( GameConfig Game )
        {
            return( Game.Configuration == Configuration && Game.Platform == Platform );
        }

        public void DeleteCutdownPackages( Main Parent )
        {
            Parent.DeleteDirectory( GameName + "/CutdownPackages", 0 );
        }

        private string GameName;
        private string Platform;
        private string Configuration;
    }
}
