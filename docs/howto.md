# How to

## Settings

You have to define all fields to make sure everything will work later. An Octopart key you can get (here) and is important to send queries to Octopart and fetch data from [here](https://octopart.com/api/register).

Make sure there are *.lib files in the source path. From these files the symbol will be used.

The resulting library will be placed in target library path.

If you want exact query matches you can select in here. With "exact queries" queries like "Atmega328P" won't find anything. Instead you had to enter "Atmega328P-PU".

## select an Octopart query result to use for part creation
After setting up the program and entering a query you can proceed to the tab "Kicad input library tab". To do this select a row of the query result table, click a double click on the row or just click on the next tab if the query result just consists of one line.

## select a Kicad source symbol
If there are already rules for that Octopart category which define also the source part you can just click on the link e.g. "device.lib/C [rule name]". If not you have to scroll and find the corresponding device. With a selected device you can click on the next tab "Kicad lib creation".

## Fill in fields
If you used the rule link from the tab before, the matching rule is already applied and all fields are filled in as defined. If not you can select and apply the rule in the rule-combobox.


The fields ID and Datasheet are read only as they should be filled by rule.

The Footprint combobox contains all footprints found in the footprint path setting. You should select the footprint out of the footprint list because this way you make sure that the footprint actually exists. The resulting part will linked to this footprint. You can reset the footprint combobox text with a click on the link besides it.

With a click on Create the above defined part will be inserted in the library "library name".


## Rules
It is recommended to use rules as much as possible because they ease a lot the work flow and are actually the reason why this tool was written.

The best way to create a rule is via the "Kicad lib creation" tab and the edit button. If the rule window is accessed this way you can also define the Octopart category which triggers the rule. You can then also select the source symbol which will be proposed as a shortcut next time when a part with this Octopart category is used again.

Categories are set in the corresponding tabulator of the rule window. A double click on the proposed category will add it to the used categories list. Source devices work the same way. Again: only if you opened this window from the creation tab there will be proposed category/source devices.

Field values will be set up using the target panel. For each field there is a text box which you can fill with text or variables. For knowing which variables exist there is a variable button besides the text box. A window with a list of variable-value pairs appears. Double click on the variable you want to use will add it to the selected text field. A new line will be converted to a space when applied. Pre/suffixes are possible: `C_%octo.footprint%` will convert to e.g. `C_0805`.

* `%targed.id%` is the time stamp in milliseconds. Hopefully unique enough to use it as a unique key.
* `%source.ref%` is the reference of the source symbol.
* `%xxxx.nicenum%` will insert si suffixes as 5k6 or 100n without spaces
* `%xxxxx.saveFilename%` is cleaned from characters which might cause trouble with your file system. If you want to create filenames (e.g. for target libname or datasheet) you should use those variables.

It is recommended to use the global rule for fields like Datasheet, ID, MPN, and Manufacturer. The global rule overwrites all other rules if the corresponding field is set. You can see it that fields which are overwritten by the global rule are grayed out.

The tab "Keywords" is not used yet.
