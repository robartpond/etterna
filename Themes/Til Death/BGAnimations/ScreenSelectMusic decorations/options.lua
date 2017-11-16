--Local vars
local update = false
local steps
local song
local frameX = 10
local frameY = 45
local frameWidth = capWideScale(320,400)
local frameHeight = 350
local fontScale = 0.4
local distY = 15
local offsetX = 40
local labelWidth = 30
local offsetY = 40
local optionsPerColumn
local optionWidth = 150
local spacingY = 10
local sectionsWidth = 350
local sectionsFontScale = 0.85
local curSection = 1
local optionLabels = {"option1", "option2"}
local effectOptions = {"Hidden", "Sudden", "Invert", "Boomerang", "Drunk", "Boost", "Brake", "Wave", "Dizzy", "Tornado", "Tipsy", "Bumpy", "Flip", "Confusion", "Invert", "Twirl", }
local chartModifiers = {"Roll", "Shuffle", "Mirror", "SoftShuffle", "SuperShuffle", "NoMines", "NoJumps", "NoHands", "NoQuads", "NoLifts", "NoFakes"}
local speedOptions = {"Speed", "XMod", "CMod", "MMod", "OverHead",} --Speed and perspective
local bgOptions = {"Cover", "Brightness"}
local difficultyOptions = {"LifeSetting", "DrainSetting", "BatteryLives", "Judge", }
local perspective = {}
local sections = {"Perspective", "Background", "Effects", "ChartModifiers", "Difficulty"}
local options
local values = {{value = "", inputType = 0}}
local activeOpt = 0
local sectionsX = frameX+offsetX + (frameWidth-sectionsWidth)/2
local sectionsY = frameY+offsetY 
local sectionsSpacing = sectionsWidth/#sections


local function ButtonActive(self)
	return isOver(self) and update
end
local function OptionsInput(event)
	if update and activeOpt>0 and event.type ~= "InputEventType_Release" then
		if event.button == "Start" or event.button == "Back" then
			activeOpt = 0
			MESSAGEMAN:Broadcast("OptionsUpdate")
			SCREENMAN:set_input_redirected(PLAYER_1, false)
			return true
		elseif event.DeviceInput.button == "DeviceButton_backspace" then
			values[activeOpt].value = values[activeOpt].value:sub(1, -2)
		elseif event.DeviceInput.button == "DeviceButton_delete"  then
			values[activeOpt].value = ""
		elseif values[activeOpt].inputType then
			if values[activeOpt].inputType == 0 then
				for i=1,#numbershers do
					if event.DeviceInput.button == "DeviceButton_"..numbershers[i] then
						if values[activeOpt].value == "0" then 
							values[activeOpt].value = ""
						end
						values[activeOpt].value = values[activeOpt].value..numbershers[i]
					end
				end
			elseif values[activeOpt].inputType == 1 then
			
			end
		end
		if values[activeOpt].inputType and values[activeOpt].inputType == 0 and SSQuery[activebound][ActiveSS] == "" then 
			SSQuery[activebound][ActiveSS] = "0"
		end
		MESSAGEMAN:Broadcast("OptionsUpdate")
	end
end

function section(i)
	return Def.ActorFrame{
		BeginCommand=function(self)
			self:xy(sectionsX + (i-1)*sectionsSpacing, sectionsY)
		end,
		Def.Quad{
			InitCommand=function(self)
				self:zoomto(sectionsSpacing,30):diffuse(getMainColor('frames')):diffusealpha(0.35)
			end,
			SetCommand=function(self)
				if i == curSection then
					self:diffusealpha(1)
				else
					self:diffusealpha(0.35)
				end
			end,
			MouseLeftClickMessageCommand=function(self)
				if ButtonActive(self) then
					curSection = i
					MESSAGEMAN:Broadcast("UpdateOptions")
				end
			end,
			UpdateOptionsMessageCommand=function(self)
				self:queuecommand("Set")
			end	
		},
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:diffuse(getMainColor('positive')):maxwidth(sectionsSpacing):maxheight(25):zoom(sectionsFontScale)
			end,
			BeginCommand=function(self)
				self:settext(sections[i])
			end
		},
	}
end
function stringOption(i, label, xpos, ypos)
	return Def.ActorFrame{
		BeginCommand=function(self)
			self:addx(offsetX + optionWidth * math.floor(i/optionsPerColumn)):addy(offsetY + spacingY * (i%optionsPerColumn))
		end,
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:halign(0):madwidth(labelWidth):zoom(fontScale)
			end,
			SetCommand=function(self)
				self:settext( label)
			end	
		},
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:addx(labelWidth/fontScale):halign(0):zoom(fontScale)
			end,
			SetCommand=function(self)
				self:settext(values[i].value)
			end,
			OptionsUpdateMessageCommand = function(self)
				self:queuecommand("Set")
			end
		},
	}
end
function speed()
	return GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):CMod() or GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):XMod() or GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):MMod()
end
function speedmod()
	return GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):CMod() and "C" or 
		GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):XMod() and "X" or 
		GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred"):MMod() and "M"
end
--Actor Frame
local t = Def.ActorFrame{
	BeginCommand=function(self)
		self:queuecommand("Set"):visible(false)
	end,
	OffCommand=function(self)
		self:bouncebegin(0.2):xy(-500,0):diffusealpha(0)
	end,
	OnCommand=function(self)
		self:bouncebegin(0.2):xy(0,0):diffusealpha(1)
	end,
	SetCommand=function(self)
		self:finishtweening()
		if getTabIndex() == 9 then
			self:queuecommand("On")
			self:visible(true)
			
			MESSAGEMAN:Broadcast("UpdateOptions")
			update = true
		else 
			self:queuecommand("Off")
			update = false
		end
	end,
	CurrentRateChangedMessageCommand=function(self)
		self:queuecommand("Set")
	end,
	RefreshChartInfoMessageCommand=function(self)
		self:queuecommand("Set")
	end,
	TabChangedMessageCommand=function(self)
		self:queuecommand("Set")
	end,
	PlayerJoinedMessageCommand=function(self)
		self:queuecommand("Set")
	end,
}

--BG quad
t[#t+1] = Def.Quad{InitCommand=function(self)
	self:xy(frameX,frameY):zoomto(frameWidth,frameHeight):halign(0):valign(0):diffuse(color("#333333CC"))
end,}

t[#t+1] = LoadFont("Common Large")..{
	InitCommand=function(self)
		self:halign(0):zoom(fontScale)
		self:xy(frameX,frameY)
		self:settext( GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptionsString("ModsLevel_Preferred"))
	end,
	SetCommand=function(self)
		self:settext( GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptionsString("ModsLevel_Preferred"))
	end	
}
t[#t+1] = LoadFont("Common Large")..{
	InitCommand=function(self)
		self:halign(0):zoom(fontScale)
		self:xy(frameX,frameY+10)
		self:settext( speedmod() .. tostring(speed()))
	end
}


for i=1,#sections do
	t[#t+1] = section(i)
end

return t
