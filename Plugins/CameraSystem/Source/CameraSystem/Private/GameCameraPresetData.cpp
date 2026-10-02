#include "GameCameraPresetData.h"

bool UGameCameraPresetData::FindPreset(FName PresetName, FGameCameraEventerPreset& OutPreset) const
{
	if (PresetName.IsNone())
	{
		return false;
	}

	// Find preset.
	for (const FGameCameraEventerPreset& Preset : Presets)
	{
		if (Preset.PresetName == PresetName)
		{
			OutPreset = Preset;
			return true;
		}
	}

	return false;
}

bool UGameCameraPresetData::SavePreset(FName PresetName, const FGameCameraEventerPreset& Preset, bool bOverwrite)
{
	if (PresetName.IsNone())
	{
		return false;
	}

	// Update existing.
	for (int32 Index = 0; Index < Presets.Num(); ++Index)
	{
		if (Presets[Index].PresetName == PresetName)
		{
			if (!bOverwrite)
			{
				return false;
			}

			Presets[Index] = Preset;
			Presets[Index].PresetName = PresetName;

			return true;
		}
	}

	// Add new.
	FGameCameraEventerPreset NewPreset = Preset;
	NewPreset.PresetName = PresetName;

	Presets.Add(NewPreset);

	return true;
}

bool UGameCameraPresetData::DeletePreset(FName PresetName)
{
	if (PresetName.IsNone())
	{
		return false;
	}

	// Find and delete.
	for (int32 Index = 0; Index < Presets.Num(); ++Index)
	{
		if (Presets[Index].PresetName == PresetName)
		{
			Presets.RemoveAt(Index);
			return true;
		}
	}

	return false;
}