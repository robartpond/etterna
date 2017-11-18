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
local offsetY = 20
local optionsPerColumn = 13
local optionWidth = 500
local inputWidth = 500
local spacingY = 10
local sectionsWidth = 350
local sectionsHeight = 30
local sectionsFontScale = 0.85
local curSection = 1
local options
local values = {}
local activeOpt = 0

local curIndex = 0

local optObj = GAMESTATE:GetPlayerState(PLAYER_1):GetPlayerOptions("ModsLevel_Preferred")

--option = {"label", type, GetValue(options), setValue(options, value)}
--types: 0=boolean, 1 =string, 2= numeric string, 3=one of many (5th is a list of options)
local effectOptions = {
		{"Hidden", 0, function(options) return options:Hidden()>0 end, function(options, value) options:Hidden(value and 1 or 0) end}, 
		{"Sudden", 0, function(options) return options:Sudden()>0 end, function(options, value) options:Sudden(value and 1 or 0) end}, 
		{"Invert",  0, function(options) return options:Invert()>0 end, function(options, value) options:Invert(value and 1 or 0) end}, 
		{"Boomerang",  0, function(options) return options:Boomerang()>0 end, function(options, value) options:Boomerang(value and 1 or 0) end}, 
		{"Drunk",  0, function(options) return options:Drunk()>0 end, function(options, value) options:Drunk(value and 1 or 0) end}, 
		{"Boost",  0, function(options) return options:Boost()>0 end, function(options, value) options:Boost(value and 1 or 0) end}, 
		{"Brake",  0, function(options) return options:Brake()>0 end, function(options, value) options:Brake(value and 1 or 0) end}, 
		{"Wave",  0, function(options) return options:Wave()>0 end, function(options, value) options:Wave(value and 1 or 0) end}, 
		{"Dizzy",  0, function(options) return options:Dizzy()>0 end, function(options, value) options:Dizzy(value and 1 or 0) end}, 
		{"Tornado",  0, function(options) return options:Tornado()>0 end, function(options, value) options:Tornado(value and 1 or 0) end}, 
		{"Tipsy",  0, function(options) return options:Tipsy()>0 end, function(options, value) options:Tipsy(value and 1 or 0) end}, 
		{"Bumpy",  0, function(options) return options:Bumpy()>0 end, function(options, value) options:Bumpy(value and 1 or 0) end}, 
		{"Flip",  0, function(options) return options:Flip()>0 end, function(options, value) options:Flip(value and 1 or 0) end}, 
		{"Confusion",  0, function(options) return options:Confusion()>0 end, function(options, value) options:Confusion(value and 1 or 0) end}, 
		{"Invert",  0, function(options) return options:Invert()>0 end, function(options, value) options:Invert(value and 1 or 0) end}, 
		{"Twirl", 0, function(options) return options:Twirl()>0 end, function(options, value) options:Twirl(value and 1 or 0) end},  }
local chartModifiers = {
		{"Roll",   0, function(options) return options:Roll()>0 end, function(options, value) options:Roll(value and 1 or 0) end}, 
		{"Shuffle", 0, function(options) return options:Shuffle() end, function(options, value) options:Shuffle(value) end}, 
		{"Mirror", 0, function(options) return options:Mirror() end, function(options, value) options:Mirror(value) end}, 
		{"SoftShuffle", 0, function(options) return options:SoftShuffle() end, function(options, value) options:SoftShuffle(value) end}, 
		{"SuperShuffle", 0, function(options) return options:SuperShuffle() end, function(options, value) options:SuperShuffle(value) end},
		{"NoMines", 0, function(options) return options:NoMines() end, function(options, value) options:NoMines(value) end},  
		{"NoJumps", 0, function(options) return options:NoJumps() end, function(options, value) options:NoJumps(value) end},  
		{"NoHands", 0, function(options) return options:NoHands() end, function(options, value) options:NoHands(value) end},  
		{"NoQuads", 0, function(options) return options:NoQuads() end, function(options, value) options:NoQuads(value) end},  
		{"NoLifts", 0, function(options) return options:NoLifts() end, function(options, value) options:NoLifts(value) end},  
		{"NoFakes", 0, function(options) return options:NoFakes() end, function(options, value) options:NoFakes(value) end}  }
local speedOptions = {"Speed", "XMod", "CMod", "MMod", "OverHead",} --Speed and perspective
local speedMods = {"XMod", "CMod", "MMod"}
local bgOptions = {
		{"Cover", 0, function(options) return options:Cover()>0 end, function(options, value) options:Cover(value and 1 or 0) end},  
		{"Brightness", 0}}
local judgesToWindow = {1.50,1.33,1.16,1.00,0.84,0.66,0.50,0.33,0.20}
local difficultyOptions = {
		{"LifeSetting", 0}, 
		{"DrainSetting", 0}, 
		{"BatteryLives", 0}, 
		{"Judge", 2, function(options) return tostring(GetTimingDifficulty()) end, function(options, value) values[activeOpt].value = math.max(math.min(tonumber(value), 9), 1);MESSAGEMAN:Broadcast("OptionsUpdate");SetTimingDifficulty(judgesToWindow[values[activeOpt].value]) end} }

--MMod must be before XMod because if MMod is active XMod also returns true
--Handles conversion of XMod to 100's
function getSpeed()
	--SCREENMAN:SystemMessage("mmod: "..tostring(optObj:MMod()).. " xmod: "..tostring(optObj:XMod()).." cmod: " ..tostring(optObj:CMod()))
	return (optObj:MMod() or optObj:CMod() or optObj:XMod() and optObj:XMod()*100)
end
function setSpeed(sp)
	if optObj:MMod() then
		optObj:MMod(sp, 1)
	elseif optObj:CMod() then
		optObj:CMod(sp, 1)
	elseif optObj:XMod() then
		optObj:XMod(sp/100, 1)
	end
end
function getSize()
	return 100 - optObj:Mini()*100
end
function setSize(s)
	optObj:Mini((100.0-s)/100.0)
	SCREENMAN:SystemMessage(tostring(optObj:Mini()))
end
local perspectiveOptions = {
		{"SpeedMod", 3, 
			function(options) 
				return optObj:CMod() and 1 or 
				optObj:MMod() and 2 or 
				optObj:XMod() and 3
			end, 
			function(options, value) 
				if value == 1 then
					optObj:CMod(getSpeed(), 1)
				elseif value == 2 then
					optObj:MMod(getSpeed(), 1)
				elseif value == 3 then
					optObj:XMod(getSpeed()/100, 1)
				end
			end, 
			{"CMod", "MMod", "XMod"}
		},
		{"Scroll Speed", 2, function(options) return tostring(getSpeed()) end, function(options, val) setSpeed(tonumber(val)) end},
		{"Perspective", 3, 
			function(options) 
				return optObj:Overhead() and 1 or 
				optObj:Distant() and 2 or 
				optObj:Hallway() and 3 or 
				optObj:Space() and 4 or 
				optObj:Incoming() and 5
			end, 
			function(options, value) 
				if value == 1 then
					optObj:Overhead(true)
				elseif value == 2 then
					optObj:Distant(1)
				elseif value == 3 then
					optObj:Hallway(1)
				elseif value == 4 then
					optObj:Space(1)
				elseif value == 5 then
					optObj:Incoming(1)
				end
			end, 
			{"OverHead", "Distant", "Hallway", "Space", "Incoming"}}, 	
		{"Receptor Size", 2, function(options) return tostring(playerConfig:get_data(pn_to_profile_slot(PLAYER_1)).ReceptorSize) end, function(options, val) playerConfig:get_data(pn_to_profile_slot(PLAYER_1)).ReceptorSize = tonumber(val) end},}
local sections = {
		{"Perspective", perspectiveOptions}, 
		{"Background", bgOptions}, 
		{"Effects", effectOptions}, 
		{"ChartModifiers", chartModifiers}, 
		{"Difficulty", difficultyOptions} 
	}

function speedmod()
	return optObj:CMod() and "C" or 
		optObj:XMod() and "X" or 
		optObj:MMod() and "M"
end
local sectionsX = frameX+offsetX + (frameWidth-sectionsWidth)/2
local sectionsY = frameY+offsetY 
local sectionsSpacing = sectionsWidth/#sections
local optionsY = frameY+offsetY+sectionsHeight
local optionsX = frameX+offsetX
local positive = getMainColor('positive')
local negative = getMainColor('negative')

local function ButtonActive(self)
	return isOver(self) and update
end

local englishes = {"?","-",".",",","a", "b", "c", "d", "e","f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",";"}
local numbershers = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"}

local function OptionsInput(event)
	if update and activeOpt>0 and event.type ~= "InputEventType_Release" then
		if event.button == "Start" or event.button == "Back" then
			if sections[curSection][2][curIndex][4] then
				sections[curSection][2][curIndex][4](optObj, values[activeOpt].value)
			end
			activeOpt = 0
			MESSAGEMAN:Broadcast("NumericInputEnded")
			MESSAGEMAN:Broadcast("OptionsUpdate")
			SCREENMAN:set_input_redirected(PLAYER_1, false)
			return true
		elseif event.DeviceInput.button == "DeviceButton_backspace" then
			if #values[activeOpt].value > 0 then
				values[activeOpt].value = values[activeOpt].value:sub(1, -2)
			end
		elseif event.DeviceInput.button == "DeviceButton_delete"  then
			values[activeOpt].value = ""
		elseif values[activeOpt].inputType then
			if values[activeOpt].inputType == 1 then
				for i=1,#englishes do
					if event.DeviceInput.button == "DeviceButton_"..englishes[i] then
						values[activeOpt].value = values[activeOpt].value..englishes[i]
					end
				end
			elseif values[activeOpt].inputType == 2 then
				for i=1,#numbershers do
					if event.DeviceInput.button == "DeviceButton_"..numbershers[i] then
						if values[activeOpt].value == "0" then 
							values[activeOpt].value = ""
						end
						values[activeOpt].value = values[activeOpt].value..numbershers[i]
					end
				end
			end
		end
		if values[activeOpt].inputType == 2 and values[activeOpt].value == "" then 
			values[activeOpt].value = "0"
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
				self:zoomto(sectionsSpacing,sectionsHeight):diffuse(getMainColor('frames')):diffusealpha(0.35)
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
					MESSAGEMAN:Broadcast("OptionsUpdate")
				end
			end,
			OptionsUpdateMessageCommand=function(self)
				self:queuecommand("Set")
			end	
		},
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:diffuse(getMainColor('positive')):maxwidth(sectionsSpacing):maxheight(25):zoom(sectionsFontScale)
			end,
			BeginCommand=function(self)
				self:settext(sections[i][1])
			end
		},
	}
end

function stringOption(valueNum, sectionNum, sectionIndex, label, xpos, ypos, widthLabel, widthInput, setFunction)
	return Def.ActorFrame{
		BeginCommand=function(self)
			self:addx(xpos):addy(ypos):queuecommand("OptionsUpdateMessage")
		end,
		OptionsUpdateMessageCommand = function(self)
			if curSection == sectionNum then
				self:visible(true)
			else
				self:visible(false)
			end
		end,
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:halign(0):maxwidth(widthLabel*fontScale):zoom(fontScale)
			end,
			SetCommand=function(self)
				self:settext( label)
			end	
		},
		Def.Quad{
			InitCommand=function(self)
				self:addx(labelWidth/fontScale+10):halign(0):zoomto(widthInput*fontScale*fontScale, 18):diffuse(color("#000000")):diffusealpha(0.3)
			end,
			OptionsUpdateMessageCommand = function(self)
				if activeOpt == valueNum then
					self:diffuse(color("#666666")):diffusealpha(0.3)
				else
					self:diffuse(color("#000000")):diffusealpha(0.3)
				end
			end,
			MouseRightClickMessageCommand=function(self)
				if isOver(self) and curSection == sectionNum then
					if setFunction and values[valueNum].value then
						setFunction(optObj, values[valueNum].value)
					end
					activeOpt = 0
					curIndex = 0
					MESSAGEMAN:Broadcast("NumericInputEnded")
					SCREENMAN:set_input_redirected(PLAYER_1, false)
					MESSAGEMAN:Broadcast("OptionsUpdate")
				end
			end,
			MouseLeftClickMessageCommand=function(self)
				if isOver(self) and curSection == sectionNum then
					activeOpt = valueNum
					curIndex = sectionIndex
					MESSAGEMAN:Broadcast("NumericInputActive")
					SCREENMAN:set_input_redirected(PLAYER_1, true)
					MESSAGEMAN:Broadcast("OptionsUpdate")
				end
			end,
		},
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:addx(labelWidth/fontScale+10):halign(0):zoom(fontScale):maxwidth(widthInput*fontScale)
			end,
			SetCommand=function(self)
				self:settext(values[valueNum].value)
			end,
			OptionsUpdateMessageCommand = function(self)
				self:queuecommand("Set")
			end,
		},
	}
end

function multiOption(valueNum, sectionNum, sectionIndex, label, xpos, ypos, widthLabel, widthInput, options, setFunction)
	local h = Def.ActorFrame{
		BeginCommand=function(self)
			self:addx(xpos):addy(ypos):queuecommand("OptionsUpdateMessage")
		end,
		OptionsUpdateMessageCommand = function(self)
			if curSection == sectionNum then
				self:visible(true)
			else
				self:visible(false)
			end
		end,
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:halign(0):maxwidth(widthLabel*fontScale):zoom(fontScale)
			end,
			SetCommand=function(self)
				self:settext( label)
			end	
		},
	}
	if options == nil then
		return
	end
	for k=1,#options do
		h[#h+1] = Def.ActorFrame{
			Def.Quad{
				InitCommand=function(self)
					self:addx(labelWidth/fontScale+10*k+(k-1)*widthInput*fontScale):halign(0):zoomto(widthInput*fontScale, 18):diffuse(negative):diffusealpha(0.3)
				end,
				OptionsUpdateMessageCommand = function(self)
					if values[valueNum].value == k then
						self:diffuse(positive):diffusealpha(0.3)
					else
						self:diffuse(negative):diffusealpha(0.3)
					end
				end,
				MouseRightClickMessageCommand=function(self)
					if isOver(self) and curSection == sectionNum then
						values[valueNum].value = k
						if setFunction then
							setFunction(optObj, values[valueNum].value)
						end
						MESSAGEMAN:Broadcast("OptionsUpdate")
					end
				end,
				MouseLeftClickMessageCommand=function(self)
					self:queuecommand("MouseRightClick")
				end,
			},
			LoadFont("Common Large")..{
				InitCommand=function(self)
					self:addx(labelWidth/fontScale+10*k+(k-1)*widthInput*fontScale):halign(0):zoom(fontScale):maxwidth(widthInput)
				end,
				SetCommand=function(self)
					self:settext(options[k])
				end
			},
		}
	end
	return h
end

function booleanOption(valueNum, sectionNum, label, xpos, ypos, widthLabel, boxWidth, boxHeight, setFunction)
	return Def.ActorFrame{
		BeginCommand=function(self)
			self:addx(xpos):addy(ypos):queuecommand("OptionsUpdateMessage")
		end,
		OptionsUpdateMessageCommand = function(self)
			if update and curSection == sectionNum then
				self:visible(true)
			else
				self:visible(false)
			end
		end,
		LoadFont("Common Large")..{
			InitCommand=function(self)
				self:halign(0):maxwidth(widthLabel*fontScale):zoom(fontScale)
			end,
			SetCommand=function(self)
				self:settext( label)
			end	
		},
		Def.Quad{
			InitCommand=function(self)
				self:addx(labelWidth/fontScale+10):halign(0):zoomto(boxWidth, boxHeight)
				self:diffuse(values[valueNum].value and positive or negative)
			end,
			MouseLeftClickMessageCommand=function(self)
				if isOver(self) and update and curSection == sectionNum then
					values[valueNum].value = not values[valueNum].value
					self:diffuse(values[valueNum].value and positive or negative)
					if setFunction then
						setFunction(optObj, values[valueNum].value)
					end
				end
			end,
			MouseRightClickMessageCommand=function(self)
				self:queuecommand("MouseLeftClickMessage")
			end,
		},
	}
end

function speed()
	return optObj:CMod() or optObj:XMod() or optObj:MMod()
end
function speedmod()
	return optObj:CMod() and "C" or 
		optObj:XMod() and "X" or 
		optObj:MMod() and "M"
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
		SCREENMAN:GetTopScreen():AddInputCallback(OptionsInput)
	end,
	SetCommand=function(self)
		self:finishtweening()
		if getTabIndex() == 9 then
			self:queuecommand("On")
			self:visible(true)
			update = true
			MESSAGEMAN:Broadcast("OptionsUpdate")
		else 
			self:queuecommand("Off")
			self:visible(false)
			update = false
			MESSAGEMAN:Broadcast("OptionsUpdate")
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


local k = 1
for i=1,#sections do
	t[#t+1] = section(i)
	for j=1,#sections[i][2] do
		if sections[i][2][j][2] ==0 then
			table.insert(values, { value = sections[i][2][j][3] and sections[i][2][j][3](optObj), inputType = sections[i][2][j][2]})
			t[#t+1] = booleanOption(k, i, sections[i][2][j][1], optionsX + (j>optionsPerColumn and frameWidth/2 or 0), optionsY+(j>optionsPerColumn and j-optionsPerColumn or j)*20, optionWidth, 18, 18, sections[i][2][j][4])
		elseif sections[i][2][j][2] == 1 then
			table.insert(values, { value = sections[i][2][j][3] and sections[i][2][j][3](optObj) or "", inputType = sections[i][2][j][2]})
			t[#t+1] = stringOption(k, i, j, sections[i][2][j][1], optionsX + (j>optionsPerColumn and frameWidth/2 or 0), optionsY+(j>optionsPerColumn and j-optionsPerColumn or j)*20, optionWidth, inputWidth, sections[i][2][j][4])
		elseif sections[i][2][j][2] == 2 then
			table.insert(values, { value = sections[i][2][j][3] and sections[i][2][j][3](optObj) or "0", inputType = sections[i][2][j][2]})
			t[#t+1] = stringOption(k, i, j, sections[i][2][j][1], optionsX + (j>optionsPerColumn and frameWidth/2 or 0), optionsY+(j>optionsPerColumn and j-optionsPerColumn or j)*20, optionWidth, inputWidth, sections[i][2][j][4])
		elseif sections[i][2][j][2] == 3 then
			table.insert(values, { value = sections[i][2][j][3] and sections[i][2][j][3](optObj) or 1, inputType = sections[i][2][j][2], list = sections[i][2][j][5]})
			t[#t+1] = multiOption(k, i, j, sections[i][2][j][1], optionsX + (j>optionsPerColumn and frameWidth/2 or 0), optionsY+(j>optionsPerColumn and j-optionsPerColumn or j)*20, optionWidth, inputWidth/#sections[i][2][j][5], sections[i][2][j][5], sections[i][2][j][4])
		end
		
		k= k+1
	end
end

return t
