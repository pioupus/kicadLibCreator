# kicadLibCreator

KicadLibCreator is a tool which shall help you to create new parts for Kicad. It will be very helpful especially if you work with a full defined library which e.g. has a single part for each resistance value. Doing this in kicad the usual way is a very annoying task because you have to fill in fields like "manufacturer" "mpn" etc. manually.

This tool will help you to do exactly this in a very easy way. You enter a mpn in the built in Octopart query mask, select a symbol out of an existing kikad library and as a result you will get the selected kicad symbol equipped with the field values fetched from Octopart.

For saving work you can define rules which will fill in the field values automatically. Those rules are linked to Octopart part categories. With well defined rules you have full defined part and a downloaded datasheet within 3 clicks.

For using this program you have to get an Octopart apikey which you can get [here](https://octopart.com/api/register).

## How to
More info [here](docs/howto.md).

## screenshots
Octopart query
![screenshot ocotpart query](/docs/screen_query.png)

select the symbol you want to use
![screenshot kicad source symbol](/docs/screen_source.png)

and create the kicad part.
![screenshot target part](/docs/screen_target.png)

rules help you to fill in the field value
![screenshot rules](/docs/screen_rules.png)

you can now use that part in kicad
![screenshot created part](/docs/screen_kicad.png)
