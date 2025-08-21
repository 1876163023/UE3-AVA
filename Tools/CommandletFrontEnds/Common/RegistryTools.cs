using System;
using Microsoft.Win32;

namespace CommandletUtils
{
	/// <summary>
	/// Helper class for interacting with the registry for saving/restoring settings
	/// </summary>
	public class RegistryTools
	{
		// Registry key that all settings are saved in
		RegistryKey AppRegistryKey;

		/// <summary>
		/// Construct the registry tools with the name of the registry key to save settings under
		/// </summary>
		/// <param name="InApplicationName"></param>
		public RegistryTools(string ApplicationName)
		{
			// create or find the Application's reg key
			AppRegistryKey = Registry.CurrentUser.CreateSubKey("UnrealEngine3").CreateSubKey(ApplicationName);
		}

		/// <summary>
		/// Write the Key/Value pair to the application's user settings
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Value"></param>
		public void SaveSetting(string Key, string Value)
		{
			AppRegistryKey.SetValue(Key, Value);
		}

		/// <summary>
		/// Overload for easily setting integers
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Value"></param>
		public void SaveSetting(string Key, int Value)
		{
			AppRegistryKey.SetValue(Key, Value.ToString());
		}

		/// <summary>
		/// Overload for easily setting bools
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Value"></param>
		public void SaveSetting(string Key, bool Value)
		{
			AppRegistryKey.SetValue(Key, Value.ToString());
		}

		/// <summary>
		/// Get the value for this application's user setting of Key. If it hasn't been set, a default is returned
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Default"></param>
		/// <returns></returns>
		public string GetSetting(string Key, string Default)
		{
			object Value = AppRegistryKey.GetValue(Key);
			if (Value == null)
			{
				return Default;
			}
			return Value.ToString();
			
		}

		/// <summary>
		/// Overload for getting integer values
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Default"></param>
		/// <returns></returns>
		public int GetSetting(string Key, int Default)
		{
			// if there was bad data in the registry, return the default
			try
			{
				return Convert.ToInt32(GetSetting(Key, Default.ToString()));
			}
			catch (FormatException)
			{
				return Default;
			}
		}

		/// <summary>
		/// Overload for getting boolean values
		/// </summary>
		/// <param name="Key"></param>
		/// <param name="Default"></param>
		/// <returns></returns>
		public bool GetSetting(string Key, bool Default)
		{
			// if there was bad data in the registry, return the default
			try
			{
				return Convert.ToBoolean(GetSetting(Key, Default.ToString()));
			}
			catch (FormatException)
			{
				return Default;
			}
		}
	}
}
