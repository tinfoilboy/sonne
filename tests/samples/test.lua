--[[
This is a lua file which uses the same begin delimiter for line comments as block comments.
Fun! This file should have 13 total lines, 4 code lines, 6 comment lines, and 3 empty lines.
--]]

-- this should count as a comment
print("Hello World")

print("comment") -- this should not

print("funky")--[[
the above line should count as code as should the bottom
--]]print("stuff")