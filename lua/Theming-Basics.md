1. <a href="#what">What is a theme?</a>
1. <a href="#smlua">Basic SM Lua</a>
1. <a href="#metrics">metrics</a>
1. <a href="#bga">BGAnimations</a>
1. <a href="#bga">Scripts</a>
1. <a href="#graphics">Graphics</a>
1. <a href="#lang">Languages</a>
1. <a href="#sounds">Sounds</a>
1. <a href="#fonts">Fonts</a>
1. <a href="#fallback">_fallback</a>

<a name="what" />

##  What is a Theme?

Stepmania is basically the base engine of the game. You can think of a theme as a "skin" for the game which changes how it looks, while remaining more or less the same game. But themes can be very powerful, and have very interesting features that are completely theme-dependant(Like 'Til Death's customizable Gameplay feature). A theme tipically has the BGAnimations, Graphics, Scripts, Languages, Sounds and Fonts folders and a metrics.ini file. Each of these are exaplained below.


<a name="smlua" />

##  Basic SM Lua

Like most programming languages, Lua has if conditionals. They look like this:

    if something == anotherThing or ( something == thirdThing and thirdThing ~= anotherThing+number ) then
        doSomething()
    end

The for loop looks like this:

    for i=initialValue,finalValue,step do
       print("The step is optional. Defaults to 1")
    end

The while loop looks like this:

    while( true ) do
        print("This loop will run forever.")
    end

There are 2 things to note about SM Lua: Variables don't have explicit types(And also, almost everything is a table, asides from numbers, booleans, etc) and a variable can contain a function.

In SM Lua there are predefined objects called Actors. There are many types of specific Actors, but we will focus on 7(You can find the complete list <a href="https://github.com/stepmania/stepmania/blob/master/Docs/Themerdocs/actordef.txt">here</a>). You can find out more about them <a href="https://dguzek.github.io/Lua-For-SM5/">here</a>

There are some methods that practically all actors have: sleep(time), setx(),gety(),setx(number),sety(number),visible(boolean),zoom(size),queuecommand(commandname,),getChild(namestring),playcommand(commandname, parameters)

These are some common actor variables: name[string], OnCommand[function], OffComand[function], InitCommand[function], SetCommand[function]

### ActorFrame

An ActorFrame acts as a container for other Actors. They're usually used to group Actors(To, for example, hide/show them all together by using the ActorFrame) and as the return type of a lua file(Since the SM engine expects each file to return an Actor, and only one, they usually return an ActorFrame with a number of Actors inside it)

### Quad

A quad basically is an object that draws a rectangle. It has a position(x and y), a size(width and height) and a color. Here's a simple example:
    local width= 100
    local height = 100
    Def.Quad{
        Name="RedQuad",
        InitCommand=cmd(xy,SCREEN_CENTER_X,SCREEN_CENTER_Y;zoomto, width, height;diffuse, Color.Red)
    }

### Bitmap Text

These are basically labels. They usually have a font(From your /Fonts/ folder, or _fallback's /Fonts/), a size(Defined as zoom) and a text(Defined by the variable text and the methods gettext and settext)

### Sprite

A sprite is an object that's used to load and display an image. You can find more about them <a href="https://dguzek.github.io/Lua-For-SM5/Actors/Sprite.html">here</a>

### Sound

### Vertex

These are used to draw non-rectangle shapes. This is done by using a number of vertex's. For documentation refer to <a href="https://github.com/stepmania/stepmania/blob/master/Docs/Themerdocs/ScreenAMVTest%20overlay.lua">this</a>

### ActorFrameTexture


<a name="#bga" />

##  BGAnimations

This is where the SM engine looks for a screen's lua files. Each screen can have a number of different lua files. these files have the name of the screen and then the type after a space. Some of the most common ones are "ScreenName overlay.lua", " underlay.lua", " fadein.lua", " fadeout.lua" and " decorations.lua". "ScreenGameplay overlay" is an example. These files can also be placed inside a folder of the same name and be called "default.lua". Lua files can "include" another one by loading it as an actorframe(Therefore, not all the files will have the names of their respective screen)

<a name="#scripts" />

##  Scripts

All the files in /Scripts/ basically define global variables and functions(They can be used anywhere in your theme). These are usually related to player and theme settings, colors and utility functions. These files start with a number, which (I think)defines the order in which they're loaded

<a name="#graphics" />

##  Graphics

This folder is intended for image and animation files. But it usually also contains some lua files that are used to load said images(So instead of manually loading the image when you want to use it, you just load the file as an actor frame)

<a name="#lang" />

##  Languages

Each file here contains string variables. Each of these is under a "section"(Like "[OptionTitles]"). When the theme wants to display a string, it usually does it in a way that the engine looks for the corresponding variable under a specific section in the file for the selected language.

<a name="#sounds" />

##  Sounds

<a name="#fonts" />

##  Fonts

<a name="#fallback" />

##  _fallback

The _fallback theme does exactly what it's name says: When the engine can't find something in your theme, it "falls back" to this theme to find it. Think of it as the theme that defines the "default" values of everything(So if it's not found, it takes the default value). 

For example, all the contents of the files in fallback's /Scripts/ can be used in any theme.

## Branches

A branch is a variable with a function that SM calls at some point(The when depends on the Branch) that returns a string with a screen name to "ask" what is the next screen. For example, the "AfterGameplay" branch usually contains a function that returns something like "ScreenEvaluation". The default branches are defined in "_fallback/Scripts/02 Branches.lua". These branches can be "overriden", which basically means that when SM looks for that variable it will find another one defined by you in your theme. To override a branch you need to add some code to a /Scripts/ file that looks like this(This one overrides the "AfterselectMusic" branch to make "ScreenStageInformation" be skipped if themeConfig:get_data().global.FadeIn returns false):

	Branch.AfterSelectMusic = function()
		if SCREENMAN:GetTopScreen():GetGoToOptions() then
			return SelectFirstOptionsScreen()
		else
			if themeConfig:get_data().global.FadeIn then
				return "ScreenStageInformation"
			else
				return IsRoutine() and "ScreenGameplayShared" or "ScreenGameplay"
			end
		end
	end

## Singletons

A singleton is a thing that's sometimes done in Object Oriented Programming. We call a singleton an object that is unique and global. This means that there's only one of it, and that it's accesible from anywhere in the code. There are singletons in the C++ SM source, and there are some <a href="https://github.com/Nickito12/etterna/blob/787377f7732e8db42b2764fdaa7b049d090dcee5/Docs/Luadoc/Lua.xml#L2115">singletons available in SM Lua</a>(Like GAMESTATE and SCREENMAN). 

## Review

So, SM themes have a metrics file which defines a TON of variables. These define stuff like the position of standard objects, some characteristics of the musicwheel and all the options available in the options menu before a song. 

They also have a bunch of lua files, many of which are associated with a specific screen and are included/executed in that specific screen which are in the BGAnimations folder, some can be found in /Scripts/ which are global, and some can be found in the other folders for various purposes.

You can change which screens lead where overriding branches in any /Scripts/ file.

If you can't find something in your theme, it's probably in the _fallback theme, since that's where SM's engine looks for whatever it can't find in your theme, or in /Scripts/.
