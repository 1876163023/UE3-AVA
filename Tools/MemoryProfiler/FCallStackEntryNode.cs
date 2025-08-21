using System;
using System.Collections.Generic;
using System.Text;

namespace MemoryProfiler
{
	/**
	 * This class represents a call stack entry in the call graph.
	 */
	public class FCallStackEntryNode : FTreeListViewItem
	{
		/** Constructor.
		 * 
		 * @param	Text	The text to display.
		 */
		public FCallStackEntryNode(string Text)
			: base(Text)
		{
		}

		/** Event handler for when the item's parent has changed.
		 */
		protected override void OnParentChanged()
		{
			base.OnParentChanged();

			if(Parent != null)
			{
				CalculateColors();
			}
		}

		/** Calculates the background and foreground colors based on node depth.
		 */
		void CalculateColors()
		{
			int Count = 0;
			FTreeListViewItem Node = Parent;

			while(Node != null)
			{
				++Count;
				Node = Node.Parent;
			}

			int ColorScale = Math.Min(Count * 20, 255);
			ColorScale = 255 - ColorScale;

			if(ColorScale > 140)
			{
				this.ForeColor = System.Drawing.Color.Black;
			}
			else
			{
				this.ForeColor = System.Drawing.Color.White;
			}

			BackColor = System.Drawing.Color.FromArgb(ColorScale, ColorScale, ColorScale);
		}
	}
}
