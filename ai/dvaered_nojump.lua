include("ai/dvaered.lua")

mem.armour_run = 0

function donothing ()
    ai.brake()
end

function idle () 
    ai.pushtask(0, "donothing") 
end