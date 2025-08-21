using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace UnrealConsole
{
	/// <summary>
	/// Dialog box for finding text within a <see cref="RichTextBox"/>.
	/// </summary>
	public partial class FindDialog : Form
	{
		RichTextBox TxtBox;
		int UpSearchStart;
		int DownSearchStart;

		/// <summary>
		/// Gets/Sets the text box associated with the find dialog box.
		/// </summary>
		public RichTextBox TextBox
		{
			get { return TxtBox; }
			set
			{
				DisconnectEvents();

				TxtBox = value;

				ConnectEvents();
			}
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		public FindDialog()
		{
			InitializeComponent();

			this.Combo_SearchString.KeyPress += new KeyPressEventHandler(Combo_SearchString_KeyPress);
		}

		/// <summary>
		/// Event handler for when a key has been pressed on the search string combo box.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void Combo_SearchString_KeyPress(object sender, KeyPressEventArgs e)
		{
			if(e.KeyChar == '\r')
			{
				ExecuteFindNext();
			}
		}

		/// <summary>
		/// Event handler for when the close button has been clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Button_Close_Click(object sender, EventArgs e)
		{
			Visible = false;
		}

		/// <summary>
		/// Event handler for when the Find Next button has been clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Button_FindNext_Click(object sender, EventArgs e)
		{
			ExecuteFindNext();
		}

		/// <summary>
		/// Executes a find operation.
		/// </summary>
		void ExecuteFindNext()
		{
			if(this.Combo_SearchString.Text.Length > 0)
			{
				for(int i = 0; i < Combo_SearchString.Items.Count; ++i)
				{
					string Item = Combo_SearchString.Items[i] as string;

					if(Item != null && Item == Combo_SearchString.Text)
					{
						Combo_SearchString.Items.RemoveAt(i);
						break;
					}
				}

				Combo_SearchString.Items.Insert(0, Combo_SearchString.Text);

				if(Radio_Up.Checked)
				{
					FindUp();
				}
				else
				{
					FindDown();
				}
			}
		}

		/// <summary>
		/// Searches down within the text box for the specified search string.
		/// </summary>
		void FindDown()
		{
			if(TxtBox != null)
			{
				if(TxtBox.Find(Combo_SearchString.Text, DownSearchStart, -1, GetSearchFlags()) == -1)
				{
					if(TxtBox.Find(Combo_SearchString.Text, 0, -1, GetSearchFlags()) == -1)
					{
						MessageBox.Show(this, "The specified search string does not exist!", Combo_SearchString.Text);
					}
				}
			}
		}

		/// <summary>
		/// Searches up within the text box for the specified search string.
		/// </summary>
		void FindUp()
		{
			if(TxtBox != null)
			{
				if(TxtBox.Find(Combo_SearchString.Text, 0, UpSearchStart, GetSearchFlags()) == -1)
				{
					if(TxtBox.Find(Combo_SearchString.Text, 0, TxtBox.TextLength, GetSearchFlags()) == -1)
					{
						MessageBox.Show(this, "The specified search string does not exist!", Combo_SearchString.Text);
					}
				}
			}
		}

		/// <summary>
		/// Returns the search flags for the find operation.
		/// </summary>
		/// <returns>The flags used to narrow the searching.</returns>
		RichTextBoxFinds GetSearchFlags()
		{
			RichTextBoxFinds Flags = RichTextBoxFinds.None;

			if(CheckBox_MatchCase.Checked)
			{
				Flags |= RichTextBoxFinds.MatchCase;
			}

			if(CheckBox_MatchWord.Checked)
			{
				Flags |= RichTextBoxFinds.WholeWord;
			}

			if(Radio_Up.Checked)
			{
				Flags |= RichTextBoxFinds.Reverse;
			}

			return Flags;
		}

		/// <summary>
		/// Connects event handlers to the text box being associated with the find dialog.
		/// </summary>
		void ConnectEvents()
		{
			if(TxtBox != null)
			{
				UpSearchStart = TxtBox.TextLength - 1;
				DownSearchStart = 0;

				TxtBox.Disposed += new EventHandler(TxtBox_Disposed);
				TxtBox.SelectionChanged += new EventHandler(TxtBox_SelectionChanged);
			}
		}

		/// <summary>
		/// Disconnects event handlers from the text box previously associated with the find dialog.
		/// </summary>
		void DisconnectEvents()
		{
			if(TxtBox != null)
			{
				TxtBox.Disposed -= new EventHandler(TxtBox_Disposed);
				TxtBox.SelectionChanged -= new EventHandler(TxtBox_SelectionChanged);
			}
		}

		/// <summary>
		/// Event handler for when the selectd text in the text box currently associated with the find dialog has changed.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TxtBox_SelectionChanged(object sender, EventArgs e)
		{
			if(TxtBox != null)
			{
				UpSearchStart = TxtBox.SelectionStart;
				DownSearchStart = TxtBox.SelectionStart + TxtBox.SelectionLength;			}
		}

		/// <summary>
		/// Event handler for when the text box associated with the find dialog has been disposed.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TxtBox_Disposed(object sender, EventArgs e)
		{
				DisconnectEvents();
				this.TextBox = null;
		}

		/// <summary>
		/// Event handler for the closing event.
		/// </summary>
		/// <remarks>Cancels the closing event and hides the window instead of destroying it.</remarks>
		/// <param name="e">Information about the event.</param>
		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			
			// Cancel the closing event and make the dialog invisible
			e.Cancel = true;
			this.Visible = false;
		}
	}
}