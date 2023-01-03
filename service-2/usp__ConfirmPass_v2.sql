USE [TCDB_Check]
GO
/****** Object:  StoredProcedure [dbo].[usp__ConfirmPass_v2]    Script Date: 19.05.2022 22:17:31 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[usp__ConfirmPass_v2]
(
@gateMAC nvarchar(17),
--@direction nvarchar(3),
@direction int,
@barcode nvarchar(max),
@result int output--,
)
AS
BEGIN
--declare @desc nvarchar(50)
	begin try
		set @result = 0
		DECLARE @userID uniqueidentifier set @userID = null
		declare @state int set @state = 0
--insert into dbo.test_log values (@gateMAC, @direction, @barcode)
		-- Проверяем турникет по таблице
		declare @gateId UNIQUEIDENTIFIER
		declare @IdEvent UNIQUEIDENTIFIER
		--select @gateId = dbo.Gate.IdGate from dbo.Gate where GateMAC=@gateMAC and (CodeGateState=10 or CodeGateState=20)
		--if @gateId is null
		--	begin
		--		set @result = 2000
		--		set @state = 106  --set @desc = N'Турникет не зарегистрирован!'
		--	end
		--ELSE
			BEGIN
			DECLARE @dateTime datetimeoffset(7)	set @dateTime		= SYSDATETIMEOFFSET ()
				-- Найдем билет
				declare @IdTicket UNIQUEIDENTIFIER
				declare @CodeTicketState int set @CodeTicketState = 0				
				declare @predCodeTicketState int set @predCodeTicketState = 0
				select @IdEvent = ce.IdEvent, @IdTicket = IdTicket, @predCodeTicketState = CodeTicketState
					from dbo.CheckTicket ct
						JOIN dbo.CheckEvent ce ON ce.IdEvent = ct.IdEvent
						where TicketNumber = @barcode
										AND ce.EntryStartTime < @dateTime
										AND ce.EntryStopTime > @dateTime
							--and @dateTime between ce.EntryStartTime and ce.EntryStopTime
				set @state = 200
				IF @direction = 1
						set @CodeTicketState = 10 --set @desc = N'Вход совершен'
				ELSE IF @direction = -1
						set @CodeTicketState = 20 --set @desc = N'Выход совершен'
				if @predCodeTicketState = 0
					set @result = 0             -- Новый вход
				else
					set @result = @direction

				UPDATE dbo.CheckTicket 
				SET 
					CodeTicketState = @CodeTicketState,
					PassCount = PassCount + @direction
					WHERE IdTicket = @IdTicket  --TicketNumber = @barcode
						and IdEvent = @IdEvent  --сильно ускоряет
				IF @@ROWCOUNT = 0
					BEGIN -- Проверяем наличие бабки контролера в списке юзеров
						select @userID = cht.CheckUser.ID from cht.CheckUser where cast([key] as varchar(36)) = @barcode
						if @userID is null
							BEGIN
								set @result = 1000 --set @desc = N'Проход невалидного билета!!!'
								set @state = 201
							END
						ELSE
							begin
								set @result = 1 - @CodeTicketState/10 --set @desc = N'Сервисный ' + @desc
								set @state = 200
							end
					END
			END
		
		insert into dbo.Log2( gateId, userID, spectatorKey, logTime, direction, [state2] )
			values( @gateId, @userID, @barcode, SYSDATETIMEOFFSET(), @direction, @state)

		---

		--Return 10

	end try
	begin catch

	SELECT  
			ERROR_NUMBER() AS ErrorNumber  
			,ERROR_SEVERITY() AS ErrorSeverity  
			,ERROR_STATE() AS ErrorState  
			,ERROR_PROCEDURE() AS ErrorProcedure  
			,ERROR_LINE() AS ErrorLine  
			,ERROR_MESSAGE() AS ErrorMessage;  
	end catch
END
