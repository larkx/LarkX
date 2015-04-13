angular.module("app").controller "CreateAssetController", ($scope, $location, $stateParams, $modal, Growl, BlockchainAPI, RpcService, Utils, Info, Blockchain, WalletAPI) ->
    $scope.name = $stateParams.name
    $scope.reg_fee =
        symbol: ''
        short: 0
        long: 0
        current: 0

    Info.refresh_info().then ->
        Blockchain.get_asset(0).then (v)->
            $scope.asset_reg_fee = Utils.formatAsset(Utils.asset(Info.info.asset_reg_fee, v))

    $scope.create_asset =
        symbol: ""
        name: ""
        description: ""
        memo: ""
        maximum_share_supply: "100,000,000"
        current_share_supply: 0
        precision: "1,000,000"
        issuer: $scope.name

    # TODO validate that this symbol has not already been created
    $scope.is_valid_symbol = (value) ->
        return false unless angular.isString value
        pattern = /^([A-Z0-9]{3,8})$/
        return !!value.match(pattern)

    $scope.create = ->
        form = @create_asset_form
        asset = $scope.create_asset
        asset.fee = $scope.reg_fee.current
        asset.fee_symbol = $scope.reg_fee.symbol
        @create_asset_form.$error.message = null
        maximum_share_supply = Utils.parseInt($scope.create_asset.maximum_share_supply)
        precision = Utils.parseInt($scope.create_asset.precision)
        if maximum_share_supply * precision > 1000000000000000
            @create_asset_form.$error.message = "You need to specify a lower precision or fewer shares."
            return
        $modal.open
            templateUrl: "dialog-asset-confirmation.html"
            controller: "DialogWithDataController"
            resolve:
                data: -> asset
                action: ->
                    ->
                        WalletAPI.asset_create(asset.symbol, asset.name, $scope.name, asset.description, maximum_share_supply, precision, asset.memo, false).then (response) ->
                            Growl.notice "", "Asset '#{asset.symbol}' was created"
                            copy = angular.copy(asset)
                            copy.registration_date = new Date()
                            copy.maximum_share_supply = maximum_share_supply * precision
                            copy.precision = precision
                            $scope.add_asset(copy)
                            $scope.create_asset.symbol = ""
                            $scope.create_asset.name = ""
                            $scope.create_asset.description = ""
                            $scope.create_asset.memo = ""
                            $scope.create_asset.maximum_share_supply = "100,000,000"
                            $scope.create_asset.precision = "1,000,000"
                            $scope.clear_form_errors(form)

    $scope.utils = Utils

    Blockchain.get_info().then (data) ->
        Blockchain.get_asset(0).then (asset) ->
            $scope.reg_fee.symbol = asset.symbol
            $scope.reg_fee.short = data.short_symbol_asset_reg_fee / asset.precision
            $scope.reg_fee.long = data.long_symbol_asset_reg_fee / asset.precision

    $scope.symbol_changed = ->
        return if not $scope.create_asset.symbol
        $scope.reg_fee.current = if $scope.create_asset.symbol.length < 6 then $scope.reg_fee.short else $scope.reg_fee.long
