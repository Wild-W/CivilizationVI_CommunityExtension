include("InstanceManager");

function CreateWatermark()
	local loadScreenContext = ContextPtr:LookUpControl("/InGame/TopPanel/RightContents");
	InstanceManager:new("CEWatermark", "CEWatermarkButton", loadScreenContext):GetInstance().CEWatermarkButton:SetHide(false);
	print("Community Extension watermark placed!");
end

Events.LoadGameViewStateDone.Add(CreateWatermark)