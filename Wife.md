Wife is a rewrite of the existing Stepmania 5 scoring system as well as the score save/load system designed specifically for keyboard 4k play. The first aspect is a new scoring system. It is meant to be a more accurate replacement for the default DP scoring system, the  which has fairly significant flaws. The second aspect is a consistent recordkeeping system.

## The DP scoring system and its flaws

DP point assignment is based on the successive timings windows on Judge 4: 22.5 ms, 45 ms, 90 ms, 135 ms, and 180 ms. Any judgments made within that interval have the same point assignment when the end score is calculated. This means that a 46 ms great is worth the same number of points as a 90 ms great, or 1 point.

This doesn't make any sense from a logical standpoint, and the problems are only exacerbated as you continue further through the windows. A 91 ms judgment is assigned 0 points, as is a 135 ms, but hit 1 ms later and you are now penalized 4 points (-4). Hit a note at 180 ms and you are penalized the same amount, but hit it at 181 ms (aka a miss) now penalizes you 8 points instead of 4. Essentially the intervals are too large and the change in points too drastic, creating breakpoints at which players can either be significantly more punished or rewarded based on hitting 2ms earlier/later.

The practical reality is that players are rewarded by the scoring system for intentionally half-mashing sections they aren't sure they'll be able to reliably get a full combo on. In the off chance the player overwhelmingly gets Goods and Greats, he comes out much further ahead on average than he would if he missed a couple notes trying to hit it properly. Now because the scoring system does not align with our perceptions of what takes more skill in the game, most players have developed their own quick internal algorithm for judging scores.

## Solving the scoring system problem

So the question becomes, can we come up with a scoring system that more accurately evaluates a player's performance? The answer is yes. The solution is just to not group judgments into arbitrary windows and to evaluate them on the base millisecond deviance values in the first place. Millisecond deviance is the distance in milliseconds the player hits a note relative to where it was supposed to be hit. It's simple enough to just fit those values through an equation that spits out a point value for each ms value.

So, what does that curve look like? Well let's start with what DP looks like. DP is not the most robust or accurate scoring system however it's generally pretty good.


![DP Scoring](http://puu.sh/qH68Y/026dd0f1ea.png)

(The lowest line is the miss weight, not technically assigned to a deviance value)

And now let's add a curve, say, y=1-x^2 (scaled) for simplicity's sake.

![DP Scoring with Curve](http://puu.sh/qH68D/ac41af9416.png)

We've already asserted that one of the major issues with the current system is that Goods aren't punished enough to prevent players from trying to mash things to improve their score. The differential between the Miss penalty and the Boo penalty is too great, but that's a result of the use of windows to calculate points. Now, intentionally or not, the DP system very closely resembles this curve, and if we made the very reasonable adjustment of moving Boo weights to -1 we would actually have something that almost exactly fits the curve.

This actually gives us a lot to work with. Now we don't want to curve out beyond the 180 ms window, because misses don't have attached deviance values. So we want to emulate the curve y=1-x^2 and want to curve out to 180 ms, such that we start at 2 points and end roughly around -8, or the current Miss penalty. The latest possible Boo will result in the same penalty as a Miss. This also assumes we're working with Judge 4. This is all what lays out the foundations for what is tentatively called MSS, or millisecond scoring.

## Millisecond Scoring

The idea is to as closely emulate DP while negating its shortcomings, so many of these values should be 'familiar'.

* CurveBegin = 18 ms - Deviance values below this point are assigned max points (2). Think of this as the new "Perfect Window"
* CurveEnd = 150 ms - Deviance values above this point are assigned the max penalty, or Miss weight. MinaciousGrace designed this to accommodate Judge 5 so anything beyond ~150ms is considered to be a miss.
* linFac = 9.5 - Linear strength of the curve.
* expFac = 2 - Exponential strength of the curve.
* MaxPoints = 2 - Maximum point value per judgment.
* MaxPenalty = -7.5 - The maximum penalty. This can also be derived by subtracting the linear factor from maximum points.

Curve Function: MaxPoints - (linFac * ((x - CurveBegin)/(CurveEnd - CurveBegin)^expFac)

And essentially it looks like this:

![MSS Curve](http://puu.sh/qH8ps/7e6da107cd.png)

And here are the actual values at each integer deviance value: http://pastebin.com/UAQgifcB

The goal was roughly to set the midpoint of each interval on the curve more or less to the Judge 5 DP point value/window counterparts, with the exception of Goods which are punished much more significantly. Goods aside, if you average the same number of judgments in the first half of each interval as the second half, you'll see a negligible difference in your resulting score from Judge 5 DP. Misses are punished slightly less severely and the Boo penalty is slightly higher than the previous value at the earliest edge of the window and significantly more severe at the latest ends.

Now that we have a curve for scoring millisecond deviance values, we need to make a distinction between timed judgments and non-timed judgments, where timed judgments are any judgments that have an associated ms value and non-timed judgments, clearly, do not. Non timed judgements are comprised of misses/mine hits/holds and whatever other junk is in the game. 

## Freezes and scoring them

One of the less significant issues with DP scoring is the ridiculous weight placed on Freezes. It makes very little sense for held Freezes to allot a full 6 points to players. Take [Rebirth's chart for Blue Planet](https://www.youtube.com/watch?v=AsCS3wJLPLs) for example. Consider that despite having nearly 1800 "taps", the 483 holds of the chart make up about 45% of the total points for the file.

Now Freeze pattern difficulty in Stepmania is largely a function of how difficult is it to retain a Freeze hold while still hitting whatever else is happening on top of it. In most charts, there's virtually nothing going on and Freezes are used for accenting specific sounds in charts rather than how they affect how charts are played.

It makes very little sense in this system to award points for held Freezes rather than for penalizing points for missed freezes, as you end up giving players free points for doing nothing. This reduces the difficulty of the chart to "score" on without reducing the physical demand for the chart.

What Wife does instead is to award 0 points for held Freezes and penalize 5 or so points for missing a Freeze. They would operate much more like Mines and this would correct the nonsensical nature of their involvement in the scoring system.

## Recording scores with Charts in a consistent manner

The second main feature Wife has is the recording of scores for songs regardless if the simfile for that song was modified in anyway. To understand this, we must elaborate on how vanilla Stepmania records scores and stores them.

In Stepmania, steps are inherently attached to their parent songs; songs are ID'd by their path in your Songs folder. The result is that scores obtained by playing these songs persist in the high score table even if the chart has fundamentally been altered, whether by adding or removing notes, or changing pattern formations. Conversely, the same songs with the same steps in different directories have their own separate entries in the high score table. Changing the name of the song group folder, which contains these songs, in any way effectively deletes any score entries you've obtained on the given simfiles. The same applies for simfiles that are contained in WIP or re-release versions of  song packs, even if the charts are unaltered by time of official release.

As the game treats it, a simfile is nothing more than a set of instructions given to players through the game, with some additional metadata and an audio file playing in the background. A chart in the Wife system is defined as only a unique set of instructions, independent from the "song", "steps", and any metadata. It is simply the notes you had to play, in the order you had to play them in, and at the points in time you had to play them at. This information is converted into a string and then hashed to produce an ID for a specific chart. Any .sm files with the same chartkey share entries in the Wife structure. For example, if you copy the song Paranoia from a folder named "DDR" into another folder named "IIDX", Wife will identify that as the same Paranoid that was in the "DDR" folder and all copies of it will share the same score tables.

Now let's try another scenario. Imagine that you do the same thing with the song Quasar. However, in the process you delete the MP3 file for Quasar, move the MP3 for Red Zone into the Quasar folder, change the music path in the Quasar .sm to the filename for the Red Zone MP3, and then change the folder name to Tamayura. Just because the MP3 is now "Red Zone", and the folder is now "Tamayura" doesn't mean you're not playing Quasar. Wife will recognize that and the chart for Quasar will again share the same entry as the Quasar file in "DDR".

However replacing the .sm file for Quasar with an .sm file for Ageha will produce a different key. In fact, it would produce the key for Ageha, or rather, the key for the set of instructions from which the relatable chart for Ageha has been constructed. Altering any set of metadata will have no effect on the key generated. Up to a certain point, altering BPMs will also have no effect. Simfiles with identical steps but different BPMs like Vertex Beta and Vertex Gamma will not share the same key. However, a simfile with static BPM edited and saved with slight BPM flux around the original static BPM, and then edited saved again somewhere else but instead having only a single static BPM again will share the same key. The tolerance level is currently around 1ms, if any notes stray further than that different keys will be produced.

The idea this feature is: if it plays the same, it is the same. The goal was to produce something robust enough to catch any "real" (i.e. affecting gameplay on a practical level) changes in a chart but flexible enough to ignore anything else.

## Playing with other Judge difficulties

Another major distinction between Wife and the scoring system in vanilla Stepmania is that Wife records your millisecond deviance record for anything you play along with your judgments and any other standard data output.

Despite MSS being designed for Judge 5, you can still use it while playing on Judge 4, though it will weigh anything in the 151-180 ms range as a Miss (basically the majority of the Boo window). This isn't particularly desirable for players not used to playing on Judge 5 and switching to MSS might be too daunting or even just frustrating. To mitigate this, the entire score calculation system has been rewritten to calculate based on standardized input designed to accommodate millisecond based scoring. This means that any number of scoring systems are supported whether they emulate MSS or DP in methods of calculation ad infinitum. This means it's easy to support a Judge 4 targeted variant of MSS that looks something like this:

* CurveBegin = 40 ms
* CurveEnd = 180 ms
* linFac = 9.5
* expFac = 2
* MaxPoints = 2
* MaxPenalty = -7.5

In Etterna, this scoring type is called **Waifu**, a derivative of Wife.

Having to deal with multiple scoring types is not a problem, since the original millisecond deviance record is being stored and the score calculation is standardized. Wife will calculate and store percent values for each type at the evaluation screen. It will then only display the value for whichever scoring type you have currently selected (along with indicating what that type is). Switching from Waifu to Wife will simply change which value is displayed. So will switching to and from any combination of DP/MIGS/Wife and so on and so forth.

The Judge value under which scores were achieved on are saved as well as the parameters. Wife will check for changes in either; changing from Judge 4 to Judge 5 will recalculate DP/MIGS/Waifu. Changing the parameters for any of the millisecond based systems will also invoke a recalculation using the new parameters.

This only applies to scores attained after Wife is operating. Pre-existing scores in the game's high score table are converted into Wife's format. The percent values for each scoring type are calculated with the assumption that they are achieved on Judge 4, millisecond based scoring systems will be ignored, and nothing will be calculated. 

### Other Links 

[Spreadsheet showing the difference between Wife and DP.](https://docs.google.com/spreadsheets/d/15efgvF256nn1Z91djn-2e77kgMAq9sQy1DWfOA1DuKw/edit#gid=1537673854)