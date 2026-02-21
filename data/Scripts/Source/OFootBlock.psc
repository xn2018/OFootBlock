ScriptName OFootBlock Extends OStimAddon

OsexIntegrationMain ostim
Actor[] ActorArrays
;=================================================
; initialization
;=================================================
Event OnInit()
	; install addon(required)
	ostim = OUtils.GetOStim()
	InstallAddon("OFootBlock")
	RegisterForModEvent("ostim_thread_start", "OStimThreadStart")
	RegisterForModEvent("ostim_thread_end", "OStimThreadEnd")
	Debug.Notification("OFootBlock init finish")
EndEvent

;=================================================
; OStim Event callback
;=================================================

Event OStimThreadStart(string EventName, string StrArg, float ThreadID, Form Sender)
	int curentThreadID = ThreadID as Int
	ActorArrays = OThread.GetActors(curentThreadID)
	SendSceneActorsToCPP("ofb_thread_start", ActorArrays)
EndEvent

Event OStimThreadEnd(string EventName, string StrArg, float ThreadID, Form Sender)
	SendSceneActorsToCPP("ofb_thread_end", ActorArrays)
EndEvent

;=================================================
; Core bridging logic
;=================================================

Function SendSceneActorsToCPP(string phase, Actor[] act)
	if !ostim
		Debug.Notification("[OFootBlock] ostim not exist")
		return
	endif

	if !act || act.Length == 0
		Debug.Notification("[OFootBlock] Unable to find actors")
		return
	endif

	;---------------------------------------------
	; strArg (C++ friendly)
	; Phase=Start|Count=2|Actors=P:14A2F,N:1C3B9
	;---------------------------------------------
	string payload = "Phase=" + phase
	payload += "|Count=" + act.Length
	payload += "|Actors="

	int i = 0
	while i < act.Length
		if act[i] == Game.GetPlayer()
			payload += "P:"
		else
			payload += "N:"
		endif

		payload += act[i].GetFormID()

		if i < act.Length - 1
			payload += ","
		endif

		i += 1
	endwhile

	
	SendModEvent(phase, payload, 0.0)
EndFunction
