#pragma once

#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class URuntimeGameplaySettingsProfile;

class FRuntimeGameplaySettingsProfileCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply HandleRefreshPropertiesClicked();
	void RefreshProfileProperties(URuntimeGameplaySettingsProfile* Profile) const;

	TArray<TWeakObjectPtr<URuntimeGameplaySettingsProfile>> CustomizedProfiles;
};
