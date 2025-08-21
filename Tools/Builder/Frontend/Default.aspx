<%@ Page Language="C#" AutoEventWireup="true" CodeFile="Default.aspx.cs" Inherits="_Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
<title>Epic Build System - Main Page</title>
<meta http-equiv="REFRESH" content="2" />
</head>
<body>
<center>

<form id="Form1" runat="server">
<asp:Label ID="Label_Title" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="XX-Large" ForeColor="Blue" Height="48px" Text="Epic Build System" Width="320px"></asp:Label>
<br />
<asp:Label ID="Label_Welcome" runat="server" Height="32px" Width="384px" Font-Bold="True" Font-Names="Arial" Font-Size="Small" ForeColor="Blue"></asp:Label>

<br />
<asp:Button ID="Button_TriggerBuild" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger Build" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTPC" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (PC Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTPS3" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (PS3 Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTX360" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (X360 Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyDelta" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Gears (Delta Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_TriggerPCS" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger PCS" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_TriggerCook" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger Cook" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_PromoteBuild" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Promote Build" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_RestartControllers" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Restart Controllers" OnClick="Button_RestartControllers_Click" />
<br />
<br />

    </center>
    <center>

<asp:SqlDataSource ID="BuilderDBSource_BuildLog" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectBuildStatus"></asp:SqlDataSource>
<asp:Repeater ID="Repeater_BuildLog" runat="server" DataSourceID="BuilderDBSource_BuildLog" OnItemCommand="Repeater_BuildLog_ItemCommand">
<ItemTemplate>

<table width="80%">
<tr><td align="center">
<asp:Label ID="Label_BuildLog1" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
is building from ChangeList :
<asp:Label ID="Label_BuilderLog2" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"ChangeList\"]") %> />
<br />
<asp:Label ID="Label_BuilderLog3" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"CurrentStatus\"]") %> />
</td><td>
<asp:Label ID="Label_BuilderLog4" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DateDiff( DataBinder.Eval(Container.DataItem, "[\"BuildStarted\"]") ) %> />
</td><td>
Triggered by : 
<asp:Label ID="Label_BuilderLog5" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Operator\"]") %> />
</td><td align="center" Width="40%">
<asp:Button ID="Button_StopBuild" runat="server" Font-Bold="True" Width="384" ForeColor="Red" Font-Size="Large" Text=<%# "Stop " + DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
</td></tr>
</table>

</ItemTemplate>
</asp:Repeater>
        &nbsp; &nbsp;

<asp:SqlDataSource ID="BuilderDBSource_JobLog" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectJobStatus"></asp:SqlDataSource>
<asp:Repeater ID="Repeater_JobLog" runat="server" DataSourceID="BuilderDBSource_JobLog">
<ItemTemplate>

<table width="80%">
<tr><td align="center">
<asp:Label ID="Label_JobLog1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
<asp:Label ID="Label_JobLog2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DateDiff2( DataBinder.Eval(Container.DataItem, "[\"BuildStarted\"]") ) %> />
:
<asp:Label ID="Label_JobLog3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"CurrentStatus\"]") %> />
</td>
</tr>
</table>

</ItemTemplate>
</asp:Repeater>
        &nbsp; &nbsp;

<br />
    </center>
    <center>
                <asp:Label ID="Label_Main" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="Main UnrealEngine3 branch" Width="640px" ForeColor="BlueViolet"></asp:Label><br />
   
<asp:SqlDataSource ID="BuilderDBSource_Commands" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectBuilds">
<SelectParameters>
<asp:QueryStringParameter Name="DisplayID" Type="Int32" DefaultValue="100" />
<asp:QueryStringParameter Name="DisplayDetailID" Type="Int32" DefaultValue="0" />
</SelectParameters>
</asp:SqlDataSource>

<asp:Repeater ID="Repeater_Commands" runat="server" DataSourceID="BuilderDBSource_Commands">
<ItemTemplate>
Last good build of
<asp:Label ID="Label_Status1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
was from ChangeList 
<asp:Label ID="Label_Status2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodChangeList\"]") %> />
on 
<asp:Label ID="Label_Status3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodDateTime\"]") %> />
<asp:Label ID="Label_Status5" runat="server" Font-Bold="True" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"DisplayLabel\"]") %> />
<asp:Label ID="Label_Status4" runat="server" Font-Bold="True" ForeColor="Green" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Status\"]") %> />
<br />
</ItemTemplate>
</asp:Repeater>

        &nbsp; &nbsp;

<br />

                <asp:Label ID="Label_UT3" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="UT3 PC/PS3/Xenon branches" Width="640px" ForeColor="BlueViolet"></asp:Label><br />

<asp:SqlDataSource ID="BuilderDBSource_Commands_UT3" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectBuilds">
<SelectParameters>
<asp:QueryStringParameter Name="DisplayID" Type="Int32" DefaultValue="300" />
<asp:QueryStringParameter Name="DisplayDetailID" Type="Int32" DefaultValue="0" />
</SelectParameters>
</asp:SqlDataSource>

<asp:Repeater ID="Repeater1" runat="server" DataSourceID="BuilderDBSource_Commands_UT3">
<ItemTemplate>
Last good build of
<asp:Label ID="Label_Status1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
was from ChangeList 
<asp:Label ID="Label_Status2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodChangeList\"]") %> />
on 
<asp:Label ID="Label_Status3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodDateTime\"]") %> />
<asp:Label ID="Label_Status5" runat="server" Font-Bold="True" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"DisplayLabel\"]") %> />
<asp:Label ID="Label_Status4" runat="server" Font-Bold="True" ForeColor="Green" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Status\"]") %> />
<br />
</ItemTemplate>
</asp:Repeater>

        &nbsp; &nbsp;

<br />
                <asp:Label ID="Label_Delta" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="Delta branch" Width="640px" ForeColor="BlueViolet"></asp:Label><br />


<asp:SqlDataSource ID="BuilderDBSource_Commands_Delta" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectBuilds">
<SelectParameters>
<asp:QueryStringParameter Name="DisplayID" Type="Int32" DefaultValue="200" />
<asp:QueryStringParameter Name="DisplayDetailID" Type="Int32" DefaultValue="0" />
</SelectParameters>
</asp:SqlDataSource>

<asp:Repeater ID="Repeater2" runat="server" DataSourceID="BuilderDBSource_Commands_Delta">
<ItemTemplate>
Last good build of
<asp:Label ID="Label_Status1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
was from ChangeList 
<asp:Label ID="Label_Status2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodChangeList\"]") %> />
on 
<asp:Label ID="Label_Status3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodDateTime\"]") %> />
<asp:Label ID="Label_Status5" runat="server" Font-Bold="True" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"DisplayLabel\"]") %> />
<asp:Label ID="Label_Status4" runat="server" Font-Bold="True" ForeColor="Green" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Status\"]") %> />
<br />
</ItemTemplate>
</asp:Repeater>

    <center>
        &nbsp;</center>

<asp:SqlDataSource ID="BuilderDBSource_Builders" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
SelectCommandType="StoredProcedure" SelectCommand="SelectActiveBuilders"></asp:SqlDataSource>

<asp:Repeater ID="Repeater_Builders" runat="server" DataSourceID="BuilderDBSource_Builders">
<ItemTemplate>
Controller
<asp:Label ID="Label_Builder1" runat="server" Font-Bold="True" ForeColor=<%# CheckConnected( DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
is available
<br />
</ItemTemplate>
</asp:Repeater>
        </form>
    </center>
   
</body>
</html>
