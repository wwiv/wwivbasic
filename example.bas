def foo(a, b)
Return a + b
enddef

a = 20
f = foo(a, 123 + 17)
a = a + 12

If a = 1 Then
PRINT("a equals 1")
ElseIf a = 23 Then
PRINT("a equals 23")
ElseIf a = 32 Then
PRINT("a equals 32")
Else
PRINT("a does not equal 1 or 23 or 32")
EndIf
