using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Data.SqlClient;
using System.Drawing;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;

public partial class _Default : BasePage
{
    protected void Page_Load( object sender, EventArgs e )
    {
        string LoggedOnUser = Context.User.Identity.Name;
        string MachineName = Context.Request.UserHostName;

        Label_Welcome.Text = "Welcome \"" + LoggedOnUser + "\" running on \"" + MachineName + "\"";
    }

    protected void Button_TriggerBuild_Click( object sender, EventArgs e )
    {
        Button Pressed = ( Button )sender;
        if( Pressed.ID == "Button_TriggerBuild" )
        {
            Response.Redirect( "Builder.aspx" );
        }
        else if( Pressed.ID == "Button_TriggerPCS" )
        {
            Response.Redirect( "PCS.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTPC" )
        {
            Response.Redirect( "UTPC.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTPS3" )
        {
            Response.Redirect( "UTPS3.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTX360" )
        {
            Response.Redirect( "UTX360.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyDelta" )
        {
            Response.Redirect( "Delta.aspx" );
        }
        else if( Pressed.ID == "Button_TriggerCook" )
        {
            Response.Redirect( "Cooker.aspx" );
        }
        else if( Pressed.ID == "Button_PromoteBuild" )
        {
            Response.Redirect( "Promote.aspx" );
        }
    }

    protected void Button_RestartControllers_Click( object sender, EventArgs e )
    {
        string CommandString;

        SqlConnection Connection = OpenConnection();

        CommandString = "UPDATE Builders SET Restart = 1 WHERE ( State != 'Dead' AND State != 'Zombied' )";
        Update( Connection, CommandString );

        CloseConnection( Connection );
    }

    protected void Repeater_BuildLog_ItemCommand( object source, RepeaterCommandEventArgs e )
    {
        if( e.Item != null )
        {
            string CommandString;

            SqlConnection Connection = OpenConnection();

            // Find the command id that matches the description
            string Build = ( ( Button )e.CommandSource ).Text.Substring( "Stop ".Length );
            CommandString = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + Build + "' )";
            int CommandID = ReadInt( Connection, CommandString );

            if( CommandID != 0 )
            {
                string User = Context.User.Identity.Name;
                int Offset = User.LastIndexOf( '\\' );
                if( Offset >= 0 )
                {
                    User = User.Substring( Offset + 1 );
                }

                CommandString = "UPDATE Commands SET Killing = 1, Killer = '" + User + "' WHERE ( ID = " + CommandID.ToString() + " )";
                Update( Connection, CommandString );
            }

            CloseConnection( Connection );
        }
    }
    
    protected string DateDiff( object Start )
    {
        TimeSpan Taken = DateTime.Now - ( DateTime )Start;

        string TimeTaken = "Time taken :\r\n";
        TimeTaken += Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" );

        return ( TimeTaken );
    }

    protected string DateDiff2( object Start )
    {
        TimeSpan Taken = DateTime.Now - ( DateTime )Start;

        string TimeTaken = "( " + Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" ) + " )";

        return ( TimeTaken );
    }

    protected Color CheckConnected( object LastPing )
    {
        if( LastPing.GetType() == DateTime.Now.GetType() )
        {
            TimeSpan Taken = DateTime.Now - ( DateTime )LastPing;

            // Check for no ping in 300 seconds
            if( Taken.TotalSeconds > 300 )
            {
                return ( Color.Red );
            }
        }

        return ( Color.DarkGreen );
    }
}
